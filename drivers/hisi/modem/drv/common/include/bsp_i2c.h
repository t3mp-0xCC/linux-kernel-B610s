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

#ifndef __BSP_I2C_H__
#define __BSP_I2C_H__

#include "bsp_om.h"

#include <product_config.h>

/*types*/
#define I2C_ERROR (-1)
#define I2C_OK (0)
/*log*/
#define  i2c_print_error(fmt, ...)    (bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_I2C, "[i2c]: <%s> "fmt, __FUNCTION__, ##__VA_ARGS__))
#define  i2c_print_info(fmt, ...)    (bsp_trace(BSP_LOG_LEVEL_INFO, BSP_MODU_I2C, "[i2c]: <%s> "fmt, __FUNCTION__, ##__VA_ARGS__))

#define I2C_READ_FLAG		0x01/*read flag*/
#define I2C_WRITE_FLAG		0x00/*write flag*/
#define SLAVE_ID_MASK		0x7f/*slave id mask*/
#define I2C_STATUS_TIMEOUT	10000/*time out*/

#define HI_I2C_CTRL_ALL		(0x7f)
#define HI_I2C_CTRL_I2CEN		(1<<8)
#define HI_I2C_CTRL_IRQEN		(1<<7)
#define HI_I2C_CTRL_START		(1<<6)
#define HI_I2C_CTRL_ACKEN		(1<<2)
#define HI_I2C_CTRL_ARBLOSS	(1<<1)
#define HI_I2C_CTRL_DONE		(1<<0)

#define HI_I2C_COM_ACK		(1<<4)
#define HI_I2C_COM_START		(1<<3)
#define HI_I2C_COM_READ		(1<<2)
#define HI_I2C_COM_WRITE		(1<<1)
#define HI_I2C_COM_STOP		(1<<0)

#define HI_I2C_SR_BUSBUSY 	(1<<7)
#define HI_I2C_SR_START		(1<<6)
#define HI_I2C_SR_ACKERR 		(1<<2)
#define HI_I2C_SR_ABITR 		(1<<1)
#define HI_I2C_SR_INTDONE 	(1<<0)

#define HI_I2C_ICR_ALLMASK	(0x7f)
#define HI_I2C_ICR_START		(1<<6)
#define HI_I2C_ICR_ACK_ERR	(1<<2)
#define HI_I2C_ICR_INTDONE_ERR	(1<<0)

#define HI_I2C_SCL_H_NUM		0x77
#define HI_I2C_SCL_L_NUM		0x77

enum i2c_master{
	I2C_MASTER0,
	I2C_MASTER1,
	I2C_MASTER_MAX

};

#ifdef CONFIG_DESIGNWARE_I2C

int bsp_i2c_slave_register(enum i2c_master,u8 slave_id);


s32 bsp_i2c_byte_data_send(u8 device_id,u8 addr, u8 data);



s32 bsp_i2c_word_data_send(u8 device_id,u8 addr, u16 data);



s32 bsp_i2c_byte_data_receive(u8 device_id, u8 addr,  u16 *data);



s32 bsp_i2c_word_data_receive(u8 device_id,u8 addr, u16 *data);



s32 bsp_i2c_initial(void);

#endif

#endif
