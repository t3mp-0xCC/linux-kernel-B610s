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
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS 
IS"
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

#ifndef __U_USIM_H
#define __U_USIM_H

#include <linux/usb/composite.h>
//#include "drv_usb.h"

/* max dev num (1 ~ n) */
#define USIM_MAX_DEV_NUM        2
#define CCID_USIM_USED_COUNT    0
#if CCID_USIM_USED_COUNT > USIM_MAX_DEV_NUM
    #error "CCID_USIM_USED_COUNT error"
#endif

#define USIM_MSG_BUFFER_SIZE    65544
#define USIM_MSG_HEADER_SIZE    10



#define PCSC_USIM_REQ           (17) /*PCSC命令请求消息ID*/
#define PCSC_PS_USIM_CNF        (41) /*PCSC命令回复消息ID*/
#define PCSC_USIM_POWER_ON      (0)  /*PCSC命令类型: 上电*/
#define PCSC_USIM_POWER_OFF     (1)  /*PCSC命令类型: 下电*/
#define PCSC_USIM_SIM_QUIRY     (2)  /*PCSC命令类型: SIM状态查询*/
#define PCSC_USIM_APDU_CMD      (3)  /*PCSC命令类型: APDU命令*/
#define PCSC_USIM_GET_ATR       (4)  /*PCSC命令类型: 获取ATR*/
#define PCSC_USIM_GET_PARA      (5)  /*PCSC命令类型: 获取SIM参数*/
#define PCSC_USIM_GET_DATA_RATE (6)  /*PCSC命令类型: 获取SIM传输速率*/
#define PCSC_USIM_GET_CLK_FREQ  (7)  /*PCSC命令类型: 获取SIM时钟频率*/

#define PCSC_APDU_MAX_LEN       (272)
#define PCSC_CMD_TIMEOUT        (300)


#define USIM_CMD_STR(cmd) (#cmd)

#define U_USIM_CKL_TBL_VALUE \
{\
0xa0, 0x0f, 0x00, 0x00, 0xc0, 0x12, 0x00, 0x00,\
0x70, 0x17, 0x00, 0x00, 0x40, 0x1f, 0x00, 0x00\
}

#define U_USIM_BAND_TBL_VALUE \
{\
0x00, 0x2a, 0x00, 0x00, 0x01, 0x54, 0x00, 0x00, 0x02, 0xa8, 0x00, 0x00, 0x05, 0x50, 0x01, 0x00,\
0x0b, 0xa0, 0x02, 0x00, 0x16, 0x40, 0x05, 0x00, 0x08, 0xf8, 0x01, 0x00, 0x0d, 0x48, 0x03, 0x00,\
0x67, 0x32, 0x00, 0x00, 0xce, 0x64, 0x00, 0x00, 0x9c, 0xc9, 0x00, 0x00, 0x39, 0x93, 0x01, 0x00,\
0x73, 0x26, 0x03, 0x00, 0xe7, 0x4c, 0x06, 0x00, 0xd6, 0x5c, 0x02, 0x00, 0x10, 0xf0, 0x03, 0x00,\
0x00, 0x1c, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x01, 0x70, 0x00, 0x00, 0x03, 0xe0, 0x00, 0x00,\
0x07, 0xc0, 0x01, 0x00, 0x0e, 0x80, 0x03, 0x00, 0x09, 0x30, 0x02, 0x00, 0x9a, 0x21, 0x00, 0x00,\
0x34, 0x43, 0x00, 0x00, 0x68, 0x86, 0x00, 0x00, 0xd1, 0x0c, 0x01, 0x00, 0xa2, 0x19, 0x02, 0x00,\
0x44, 0x33, 0x04, 0x00, 0x00, 0x15, 0x00, 0x00, 0x04, 0xfc, 0x00, 0x00, 0x06, 0xa4, 0x01, 0x00,\
0x33, 0x19, 0x00, 0x00, 0x6b, 0x2e, 0x01, 0x00, 0x80, 0x1f, 0x00, 0x00, 0x01, 0x3f, 0x00, 0x00,\
0x02, 0x7e, 0x00, 0x00, 0x06, 0x7a, 0x01, 0x00, 0x0a, 0x76, 0x02, 0x00, 0x00, 0x0e, 0x00, 0x00,\
0x04, 0x18, 0x01, 0x00, 0xcd, 0x10, 0x00, 0x00, 0x80, 0x0a, 0x00, 0x00, 0x03, 0xd2, 0x00, 0x00,\
0x99, 0x0c, 0x00, 0x00, 0x35, 0x97, 0x00, 0x00, 0xc0, 0x0f, 0x00, 0x00, 0x03, 0xbd, 0x00, 0x00,\
0x05, 0x3b, 0x01, 0x00, 0x66, 0x08, 0x00, 0x00, 0x14, 0x0a, 0x00, 0x00, 0x29, 0x14, 0x00, 0x00,\
0x52, 0x28, 0x00, 0x00, 0xa5, 0x50, 0x00, 0x00, 0x4a, 0xa1, 0x00, 0x00, 0x94, 0x42, 0x01, 0x00,\
0xf7, 0x78, 0x00, 0x00, 0x84, 0x1e, 0x00, 0x00, 0x09, 0x3d, 0x00, 0x00, 0x12, 0x7a, 0x00, 0x00,\
0x24, 0xf4, 0x00, 0x00, 0x48, 0xe8, 0x01, 0x00, 0x90, 0xd0, 0x03, 0x00, 0x36, 0x6e, 0x01, 0x00,\
0x5a, 0x62, 0x02, 0x00, 0x9f, 0x24, 0x00, 0x00, 0x3e, 0x49, 0x00, 0x00, 0x7c, 0x92, 0x00, 0x00,\
0xf8, 0x24, 0x01, 0x00, 0xf0, 0x49, 0x02, 0x00, 0xe0, 0x93, 0x04, 0x00, 0x74, 0xb7, 0x01, 0x00,\
0x6c, 0xdc, 0x02, 0x00, 0x58, 0x14, 0x00, 0x00, 0xb0, 0x28, 0x00, 0x00, 0x61, 0x51, 0x00, 0x00,\
0xc2, 0xa2, 0x00, 0x00, 0x85, 0x45, 0x01, 0x00, 0x0a, 0x8b, 0x02, 0x00, 0xe6, 0x96, 0x01, 0x00,\
0x6a, 0x18, 0x00, 0x00, 0xd4, 0x30, 0x00, 0x00, 0xa8, 0x61, 0x00, 0x00, 0x50, 0xc3, 0x00, 0x00,\
0xa0, 0x86, 0x01, 0x00, 0x40, 0x0d, 0x03, 0x00, 0x42, 0x0f, 0x00, 0x00, 0x1b, 0xb7, 0x00, 0x00,\
0x2d, 0x31, 0x01, 0x00, 0x4f, 0x12, 0x00, 0x00, 0xba, 0xdb, 0x00, 0x00, 0xe3, 0x16, 0x00, 0x00,\
0xc6, 0x2d, 0x00, 0x00, 0x8d, 0x5b, 0x00, 0x00, 0xa8, 0x12, 0x01, 0x00, 0xc3, 0xc9, 0x01, 0x00,\
0x2c, 0x0a, 0x00, 0x00, 0x73, 0xcb, 0x00, 0x00, 0x35, 0x0c, 0x00, 0x00, 0xa1, 0x07, 0x00, 0x00,\
0x96, 0x98, 0x00, 0x00, 0x27, 0x09, 0x00, 0x00, 0xdd, 0x6d, 0x00, 0x00, 0x71, 0x0b, 0x00, 0x00,\
0x54, 0x89, 0x00, 0x00, 0xe1, 0xe4, 0x00, 0x00\
}

#define U_USIM_FAKE_ATR \
{\
0x3B, 0x9F, 0x94, 0x80, 0x1F, 0xC7, 0x80, 0x31,\
0xE0, 0x73, 0xFE, 0x21, 0x11, 0x63, 0x3F, 0x01,\
0x11, 0x83, 0x07, 0x90, 0x00, 0xE7\
}

/* ccid slot status */
typedef struct slot_status {
    u32 current_state;
    u32 status_changed;
} slot_status_t;

struct gusim {
	struct usb_function		func;

	/* port is managed by gserial_{connect,disconnect} */
	void                    *ioport;

	struct usb_ep			*in;
	struct usb_ep			*out;

    /* 当前正提交USIM处理的命令 */
    unsigned int 	        usim_cmd;
    unsigned int 	        usim_cmd_status;
    unsigned int 	        apdu_len;
    unsigned char 	        apdu_data[PCSC_APDU_MAX_LEN];

	/* notification callbacks */
	void (*connect)(struct gusim *p);
	void (*disconnect)(struct gusim *p);

    /* handle rdr msg */
    void (*handle_msg)(struct gusim *p, void* buf, u32 size);
    void (*dump_fd)(struct gusim *p);
};


/* connect/disconnect is handled by individual functions */
int gusim_connect(struct gusim *g, u8 port_num);
void gusim_disconnect(struct gusim *g);

int gusim_get_slot_status(struct gusim *g, slot_status_t *status_array);
int gusim_abort_slot(struct gusim *g, u8 slot, u8 sequence);
int gusim_get_data_rates(struct gusim *g, void* buffer);
int gusim_get_clock_freqs(struct gusim *g, void* buffer);

int gusim_return_msg(struct gusim *g, void* buffer, u32 size);
int gusim_msg_proc(struct gusim *g);

/* interface for adp */
int gusim_ctrl_cmd(u32 port_num, u32 cmd_type,
                   u32 status, u8 *buf, u32 length);
void gusim_reg_cb(u32 port_num, pFunAPDUProcess pFun1, GetCardStatus pFun2);

struct usb_request *
gs_usim_alloc_req(struct usb_ep *ep, unsigned len, gfp_t kmalloc_flags);
void gs_usim_free_req(struct usb_ep *ep, struct usb_request *req);



#endif /* __U_USIM_H */

