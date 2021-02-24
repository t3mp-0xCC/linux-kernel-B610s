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


#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/delay.h>


#include "mdrv_udi.h"
#include <mdrv.h>


struct adp_acm_stat {
    int stat_open_err;
    int stat_open_last_err;
    int stat_read_err;
    int stat_read_last_err;
    int stat_write_err;
    int stat_write_last_err;
    int stat_ioctl_err;
    int stat_ioctl_last_err;
    int stat_wait_close;
};

struct adp_acm_context {
    char* dev_name;
    int is_open;
    atomic_t opt_cnt;
    struct adp_acm_stat stat;
};


static struct adp_acm_context adp_acm_ctx[] = {
    {"/dev/acm_ctrl",          /*UDI_USB_ACM_CTRL*/ 0, {0}, {0}},
    {"/dev/ttyGS0",            /*UDI_USB_ACM_AT*/ 0, {0}, {0}},
    {"/dev/acm_c_shell",       /*UDI_USB_ACM_SHELL*/ 0, {0}, {0}},
    {"/dev/acm_4g_diag",       /*UDI_USB_ACM_LTE_DIAG*/ 0, {0}, {0}},
    {"/dev/acm_3g_diag",       /*UDI_USB_ACM_OM*/ 0, {0}, {0}},
    {"/dev/acm_modem",         /*UDI_USB_ACM_MODEM*/ 0, {0}, {0}},
    {"/dev/acm_gps",           /*UDI_USB_ACM_GPS*/ 0, {0}, {0}},
    {"/dev/acm_a_shell",       /*UDI_USB_ACM_3G_GPS*/ 0, {0}, {0}},
    {"/dev/acm_err",           /*UDI_USB_ACM_3G_PCVOICE*/ 0, {0}, {0}},
    {"/dev/acm_voice",         /*UDI_USB_ACM_PCVOICE*/ 0, {0}, {0}},
    {"/dev/acm_skytone",         /*UDI_USB_ACM_SKYTONE*/ 0, {0}, {0}},
};
static int stat_invalid_devid = 0;
static int stat_invalid_filp = 0;

#define ADP_ACM_FILP_INVALID(filp) (!(filp)  || IS_ERR(filp) || !filp->private_data)

#ifdef CONFIG_USB_BALONG_UDC
/*****************************************************************************
* �� �� ��  : bsp_acm_open
*
* ��������  : ���豸(����ͨ��)
*
* �������  : dev_id: �豸id
* �������  : ��
* �� �� ֵ  :
*****************************************************************************/
void *bsp_acm_open(u32 dev_id)
{
    char* name;
    struct file* filp;

    if (dev_id >= sizeof(adp_acm_ctx) / sizeof(struct adp_acm_context)) {
        stat_invalid_devid++;
        return NULL;
    }
    name = adp_acm_ctx[dev_id].dev_name;

    filp = filp_open(name, O_RDWR, 0);
    if (!filp) {
        adp_acm_ctx[dev_id].stat.stat_open_err++;
        adp_acm_ctx[dev_id].stat.stat_open_last_err = (int)filp;
        return NULL;
    }
    filp->private_data = &adp_acm_ctx[dev_id];
    adp_acm_ctx[dev_id].is_open = 1;

    return filp;
}

/*****************************************************************************
* �� �� ��  : bsp_acm_close
*
* ��������  : �ر��豸(����ͨ��)
*
* �������  : handle: �豸��handle
* �������  : ��
* �� �� ֵ  : �ɹ�/ʧ��
*****************************************************************************/
s32 bsp_acm_close(void *handle)
{
    struct file* filp = (struct file*)handle;
    struct adp_acm_context* ctx;

    if (unlikely(ADP_ACM_FILP_INVALID(filp))) {
        stat_invalid_filp++;
        return -EINVAL;
    }

    ctx = (struct adp_acm_context*)filp->private_data;
    ctx->is_open = 0;

    /* wait for file opt complete */
    while(atomic_read(&ctx->opt_cnt)) {
        vfs_fsync(filp, 1);
        ctx->stat.stat_wait_close++;
        msleep(10);
    }

    filp_close(filp, NULL);
    return 0;
}

/*****************************************************************************
* �� �� ��  : bsp_acm_write
*
* ��������  : ����д
*
* �������  : handle:  �豸��handle
*             buf: ��ַ
*             size: ��С
* �������  :
*
* �� �� ֵ  : ����ֽ��� �� �ɹ�/ʧ��
*****************************************************************************/
s32 bsp_acm_write(void *handle, void *buf, u32 size)
{
    mm_segment_t old_fs;
    struct file* filp = (struct file*)handle;
    struct adp_acm_context* ctx;
    int status;
    loff_t pos = 0;

    if (unlikely(ADP_ACM_FILP_INVALID(filp))) {
        stat_invalid_filp++;
        return -EINVAL;
    }
    ctx = (struct adp_acm_context*)filp->private_data;

    atomic_inc(&ctx->opt_cnt);
    if (unlikely(!ctx->is_open || !(filp->f_path.dentry))) {
        status = -ENXIO;
        goto write_ret;
    }
    old_fs = get_fs();
    set_fs(KERNEL_DS);
    status = vfs_write(filp, (void __force __user *)buf, size, &pos);
    set_fs(old_fs);

write_ret:
    atomic_dec(&ctx->opt_cnt);
    if (status < 0) {
        ctx->stat.stat_write_err++;
        ctx->stat.stat_write_last_err = status;
    }
    return status;
}

/*****************************************************************************
* �� �� ��  : bsp_acm_read
*
* ��������  : ���ݶ�
*
* �������  : handle:  �豸��handle
*             buf: ��ַ
*             size: ��С
* �������  :
*
* �� �� ֵ  : ����ֽ��� �� �ɹ�/ʧ��
*****************************************************************************/
s32 bsp_acm_read(void *handle, void *buf, u32 size)
{
    mm_segment_t old_fs;
    struct file* filp = (struct file*)handle;
    struct adp_acm_context* ctx;
    int status;
    loff_t pos = 0;

    if (unlikely(ADP_ACM_FILP_INVALID(filp))) {
        stat_invalid_filp++;
        /* protect system running, usr often use read in while(1). */
        msleep(10);
        return -EINVAL;
    }
    ctx = (struct adp_acm_context*)filp->private_data;

    atomic_inc(&ctx->opt_cnt);
    if (unlikely(!ctx->is_open || !(filp->f_path.dentry))) {
        status = -ENXIO;
        goto read_ret;
    }

    old_fs = get_fs();
    set_fs(KERNEL_DS);
    status = vfs_read(filp, (void __force __user *)buf, size, &pos);
    set_fs(old_fs);

read_ret:
    atomic_dec(&ctx->opt_cnt);
    if (status <= 0) {
        /* protect system running, usr often use read in while(1). */
        msleep(100);
        ctx->stat.stat_read_err++;
        ctx->stat.stat_read_last_err = status;
    }
    return status;
}

/*****************************************************************************
* �� �� ��  : bsp_acm_ioctl
*
* ��������  : ����ͨ����������
*
* �������  : handle: �豸��handle
*             u32Cmd: IOCTL������
*             pParam: ��������
* �������  :
*
* �� �� ֵ  : �ɹ�/ʧ��
*****************************************************************************/
s32 bsp_acm_ioctl(void *handle, u32 cmd, void *para)
{
    mm_segment_t old_fs;
    struct file* filp = (struct file*)handle;
    struct adp_acm_context* ctx;
    int status;

    if (unlikely(ADP_ACM_FILP_INVALID(filp))) {
        stat_invalid_filp++;
        return -EINVAL;
    }
    ctx = (struct adp_acm_context*)filp->private_data;

    atomic_inc(&ctx->opt_cnt);
    if (unlikely(!ctx->is_open || !(filp->f_path.dentry))) {
        status = -ENXIO;
        goto ioctl_ret;
    }
    old_fs = get_fs();
    set_fs(KERNEL_DS);
    status = do_vfs_ioctl(filp, 0, (unsigned int)cmd, (unsigned long)para);
    set_fs(old_fs);

ioctl_ret:
    atomic_dec(&ctx->opt_cnt);
    if (status < 0) {
        ctx->stat.stat_ioctl_err++;
        ctx->stat.stat_ioctl_last_err = status;
    }
    return status;
}
void acm_adp_dump(void)
{
    int i;

    printk("stat_invalid_devid      :%d\n", stat_invalid_devid);
    printk("stat_invalid_filp       :%d\n", stat_invalid_filp);
    for (i = 0; i < sizeof(adp_acm_ctx) / sizeof(struct adp_acm_context); i++) {
        printk("==== dump dev:%s ====\n", adp_acm_ctx[i].dev_name);
        printk("is_open             :%d\n", adp_acm_ctx[i].is_open);
        printk("opt_cnt             :%d\n", atomic_read(&adp_acm_ctx[i].opt_cnt));
        printk("stat_open_err       :%d\n", adp_acm_ctx[i].stat.stat_open_err);
        printk("stat_open_last_err  :%d\n", adp_acm_ctx[i].stat.stat_open_last_err);
        printk("stat_read_err       :%d\n", adp_acm_ctx[i].stat.stat_read_err);
        printk("stat_read_last_err  :%d\n", adp_acm_ctx[i].stat.stat_read_last_err);
        printk("stat_write_err      :%d\n", adp_acm_ctx[i].stat.stat_write_err);
        printk("stat_write_last_err :%d\n", adp_acm_ctx[i].stat.stat_write_last_err);
        printk("stat_ioctl_err      :%d\n", adp_acm_ctx[i].stat.stat_ioctl_err);
        printk("stat_ioctl_last_err :%d\n", adp_acm_ctx[i].stat.stat_ioctl_last_err);
        printk("stat_wait_close     :%d\n", adp_acm_ctx[i].stat.stat_wait_close);
        printk("\n");
    }
}

#else
void* bsp_acm_open(u32 dev_id)
{
	return NULL;
}

s32 bsp_acm_close(void* handle)
{
	return 0;
}

s32 bsp_acm_write(void* handle, void *buf, u32 size)
{
	return 0;
}

s32 bsp_acm_read(void* handle, void *buf, u32 size)
{
	return 0;
}

s32 bsp_acm_ioctl(void* handle, u32 cmd, void *para)
{
	return 0;
}

void acm_adp_dump(void)
{
}
#endif


