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

#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include "product_config.h"
#include "bsp_om.h"
#include "mdrv_efuse.h"
#include "bsp_efuse.h"
#include "hi_efuse.h"


#include <soc_onchiprom.h>


int mdrv_efuse_write_huk(char *pBuf,unsigned int len)
{
    return OK;
}

int mdrv_efuse_check_huk_valid(void)
{
    return OK;
}


int mdrv_efuse_get_dieid(unsigned char* buf,int length)
{
    unsigned int i = 0;
    int ret = 0;

#if (EFUSE_DIEID_BIT < 32)
    u32 *buf_die_id = (u32*)buf;
#endif

    if(NULL == buf)
    {
        efuse_print_error("die id buf is error.\n");
        return EFUSE_ERROR;
    }

    if(length < EFUSE_DIEID_LEN)
    {
        efuse_print_error("die id lenth is error.\n");
        return EFUSE_ERROR;
    }

    memset(buf, 0, EFUSE_DIEID_LEN);

    pr_info("efuse read start group %d length %d.\n", EFUSE_GRP_DIEID, EFUSE_DIEID_SIZE);

    if(0 != (ret = bsp_efuse_read((u32*)buf, EFUSE_GRP_DIEID, EFUSE_DIEID_SIZE)))
    {
        efuse_print_error("die id read efuse error.\n");
        return READ_EFUSE_ERROR;
    }

    for(i = 0;i < EFUSE_DIEID_SIZE;i++)
    {
        pr_info("efuse end buf[%d] is 0x%x.\n", i, buf[i]);
    }
    pr_info("efuse read end ret %d\n", ret);

#if (EFUSE_DIEID_BIT < 32)
    buf_die_id[EFUSE_DIEID_SIZE-1] &= (((u32)0x1 << EFUSE_DIEID_BIT) - 1);
#endif

    return EFUSE_OK;
}



int mdrv_efuse_get_chipid(unsigned char* buf,int length)
{
    return EFUSE_OK;
}


void DRV_DIEID_TEST(void)
{
    int i = 0;
    char *buf = kzalloc(EFUSE_DIEID_LEN, GFP_KERNEL);
    if(NULL == buf)
    {
        efuse_print_info("alloc mem is fail.\n");
        return;
    }

    if(EFUSE_OK == mdrv_efuse_get_dieid(buf, EFUSE_DIEID_LEN))
    {
        for(i = 0;i < EFUSE_DIEID_LEN;i++)
        {
            efuse_print_info("0x%x \n", *(buf+i));
        }

    }
    else
    {
        efuse_print_error("DRV_GET_DIEID is fail.\n");
    }

    kfree(buf);
}


int mdrv_efuse_ioctl(int cmd, int arg, unsigned char* buf, int len)
{
    efuse_print_error("It's only a stub");

    return 0;
}



