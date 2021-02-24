#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/delay.h>

#include <linux/usb/bsp_acm.h>

#include "mdrv_udi.h"


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
    {"/dev/acm_m2m_gps",       /*UDI_USB_ACM_M2M_GPS*/ 0, {0}, {0}},
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
s32 bsp_acm_open(u32 dev_id)
{
    char* name;
    struct file* filp;

    if (dev_id >= sizeof(adp_acm_ctx) / sizeof(struct adp_acm_context)) {
        stat_invalid_devid++;
        return -EINVAL;
    }
    name = adp_acm_ctx[dev_id].dev_name;

    filp = filp_open(name, O_RDWR, 0);
    if (IS_ERR(filp)) {
        adp_acm_ctx[dev_id].stat.stat_open_err++;
        adp_acm_ctx[dev_id].stat.stat_open_last_err = (int)filp;
        return -EIO;
    }
    filp->private_data = &adp_acm_ctx[dev_id];
    adp_acm_ctx[dev_id].is_open = 1;

    return (s32)filp;
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
s32 bsp_acm_close(s32 handle)
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
s32 bsp_acm_write(s32 handle, void *buf, u32 size)
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
s32 bsp_acm_read(s32 handle, void *buf, u32 size)
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
s32 bsp_acm_ioctl(s32 handle, u32 cmd, void *para)
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
s32 bsp_acm_open(u32 dev_id)
{
	return 0;
}

s32 bsp_acm_close(s32 handle)
{
	return 0;
}

s32 bsp_acm_write(s32 handle, void *buf, u32 size)
{
	return 0;
}

s32 bsp_acm_read(s32 handle, void *buf, u32 size)
{
	return 0;
}

s32 bsp_acm_ioctl(s32 handle, u32 cmd, void *para)
{
	return 0;
}

void acm_adp_dump(void)
{
}
#endif


