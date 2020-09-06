
#include <linux/version.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <asm/uaccess.h> 
#include <linux/if.h>
#include "kiconcovert.h"

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
#include <linux/slab.h>
#endif

#include "hw_nls.c"
struct proc_dir_entry *proc_iconconvert = NULL;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36)
static int convtIoctl( struct inode *inode, struct file *flip,
                        unsigned int command, unsigned long arg )
{
#else
static long convtIoctl(struct file *flip,
                        unsigned int command, unsigned long arg )
{
#endif

    int iRet = -1;

    if (IOCTL_ICON_CONVERT == command)
    {
         iRet = HwNlsConvertIoCtl(arg);
    }
    return iRet;
}

static struct file_operations convt_fops =
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36)
    ioctl:          convtIoctl,
#else
    unlocked_ioctl: convtIoctl,
#endif
};

static void kiconconvrelease(void)
{
	remove_proc_entry(CONVERT_PROC, proc_iconconvert);
}


static int __init kiconconvInit( void )
{
    printk("\n kiconconvInit enter \n");
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36)
    /* µ±Ç°lastword */
    proc_iconconvert = create_proc_entry(CONVERT_PROC, 0, NULL);
    if (NULL == proc_iconconvert)
    {
        return -1;
    }	
    proc_iconconvert->proc_fops = (struct file_operations *)&convt_fops;
#else
	proc_iconconvert = proc_create(CONVERT_PROC, 0, NULL, &convt_fops);
    if (NULL == proc_iconconvert)
    {
        return -1;
    }	
#endif
    return 0;
}

core_initcall( kiconconvInit);
module_exit( kiconconvrelease);

