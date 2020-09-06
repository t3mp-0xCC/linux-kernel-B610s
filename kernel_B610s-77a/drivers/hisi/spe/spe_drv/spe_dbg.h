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

#ifndef __SPE_DBG_H__
#define __SPE_DBG_H__

#include "spe.h"

/* spe msg system */
#define SPE_MSG_ERR         BIT(0)
#define SPE_MSG_TRACE	    BIT(1)
#define SPE_MSG_INFO        BIT(2)
#define SPE_MSG_DBG		    BIT(3)
#define SPE_MSG_BUG		    BIT(4)
#define SPE_MSG_DUMP	    BIT(5)
#define SPE_MSG_PRINT	    BIT(6)
#define SPE_MSG_TUPLE       BIT(7)
#define SPE_MSG_CPU_SKB     BIT(8)

extern struct spe spe_balong;

#define SPE_ERR(fmt, ...)		\
	do {\
			printk("[%s:] "fmt, __func__, ##__VA_ARGS__);\
		} while (0)

#define SPE_BUG(args,condition)          \
    do {\
        if (condition)\
        {\
            printk args;\
        }\
\
        if (spe_balong.msg_level & SPE_MSG_BUG)\
        {\
            BUG_ON(condition);\
        }\
    } while (0)

#if defined(CONFIG_BALONG_SPE_DEBUG)
#define SPE_INFO(fmt, ...)		\
	do {\
		if (spe_balong.msg_level & SPE_MSG_INFO) \
			printk("[%s:] "fmt, __func__, ##__VA_ARGS__);\
		} while (0)
#define SPE_TRACE(fmt, ...)		\
	do {\
		if (spe_balong.msg_level & SPE_MSG_TRACE) \
			printk("[%s:] "fmt, __func__, ##__VA_ARGS__);\
		} while (0)
#define SPE_DBG(fmt, ...)		\
	do {\
		if (spe_balong.msg_level & SPE_MSG_DBG) \
			printk("[%s:] "fmt, __func__, ##__VA_ARGS__);\
		} while (0)

#define SPE_DUMP(fmt, ...)		\
	do {\
		if (spe_balong.msg_level & SPE_MSG_DUMP) \
			printk("[%s:] "fmt, __func__, ##__VA_ARGS__);\
		} while (0)

#define SPE_TUPLE(fmt, ...)		\
	do {\
		if (spe_balong.msg_level & SPE_MSG_TUPLE) \
			printk("[%s:] "fmt, __func__, ##__VA_ARGS__);\
		} while (0)

#define SPE_SKB_DUMP(skb)   \
    do{\
        if(spe_balong.msg_level & SPE_MSG_PRINT)\
        {\
        	int len = min((int)64, (int)(skb->len));\
            print_hex_dump(KERN_ERR, "skb dump: ", DUMP_PREFIX_ADDRESS, \
                16, 1, (skb->data), len, false);\
            printk(KERN_ERR"\n");\
        }\
    }while(0)
#else
#define SPE_INFO(fmt, ...)
#define SPE_TRACE(fmt, ...)
#define SPE_DBG(fmt, ...)
#define SPE_DUMP(fmt, ...)
#define SPE_TUPLE(fmt, ...)
#define SPE_SKB_DUMP(skb)
#endif

/* spe dbg system */
#define SPE_DBG_FWUDP               BIT(0)
#define SPE_DBG_RD_TIMESTAMP        BIT(1)
#define SPE_DBG_TD_RESULT           BIT(2)
#define SPE_DBG_RD_RESULT           BIT(3)



#define spe_msg_str() __FUNCTION__//" line "##__LINE__


/* spe debug functions for dbgfs */
void spe_set_rd_timestamp(void * rd_desc);
void spe_fw_time_show(void);
void spe_dump_td(unsigned int port_num);
void spe_dump_rd(unsigned int port_num);
void spe_print_td(unsigned int port_num, unsigned int td_pos);
void spe_print_rd(unsigned int port_num, unsigned int rd_pos);
void spe_dev_setting(void);
void spe_dev_info(void);
void spe_port_setting(unsigned int portno);
void spe_port_rd_info(unsigned int portno);
void spe_port_td_info(unsigned int portno);
void spe_ipfw_setting(void);
void spe_port_info_all(unsigned int port_num);
void spe_dump_all(void);


#endif /* __SPE_DBG_H__ */
