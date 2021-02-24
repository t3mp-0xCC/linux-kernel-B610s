/***************************************************************************
 * File Name  : flash_api.h
 *
 * Description: This file contains definitions and prototypes for a public
 *              flash device interface and an internal flash device interface.
 ***************************************************************************/

#if !defined(_KEROBJECTS_H)
#define _KEROBJECTS_H

#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/seq_file.h>


#ifdef __cplusplus
extern "C" {
#endif

#define HW_SSP_ATP_PROC                           "atp_proc"

/* 回调函数 */
/*******************************************************************************
    对象比较函数，必需返回两个对象的大小关系，如果pObjA>pObjB则返回大于0的数，pObjA<pObjB则返回小于0的数，否则返回0
    如果返回0，则表示两个对象完全一样，且不能放在同一个container中。
*******************************************************************************/
typedef int (*pf_hw_ssp_ker_obj_cmp)(void * pObjA, void * pObjB);
/* 
    使用二分法在container中通过KeyData来查询对象，必需返回大小关系
    如果KeyData大于比较的对象，就返回大于0的值，KeyData小于对象则返回小于0的值，否则返回0
*/
typedef int (*pf_hw_ssp_ker_obj_search_cmp)(void * pObj, int KeyData);

/* seq文件系统的读取接口，必需使用seq文件的接口向m输出要显示的内容 */
typedef int (*pf_hw_ssp_ker_obj_show)(struct seq_file *m, void * pData,  unsigned int Size);

/* 支持ioctl的接口 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36)
typedef int (*pf_hw_ssp_ker_obj_ioctl)(struct inode *pinode, struct file *pfile,  unsigned int cmd,  unsigned long Arg);
#else
typedef int (*pf_hw_ssp_ker_obj_ioctl)(struct file *pfile,  unsigned int cmd,  unsigned long Arg);
#endif


/* 支持write的接口 */
typedef ssize_t (*pf_hw_ssp_ker_obj_write)(struct file * file, const char __user * acBuffer, size_t buflen, loff_t * fpos);

/* 使用遍历的方式查询对象，如果找到对象，就返回非0值，返回非0继续查找 */
typedef int (*pf_hw_ssp_ker_obj_find)(void * pObj, void * pData);
/****************************************************************************************************************/

/***********************************************数据结构****************************************************/
typedef struct hw_ssp_ker_obj_container_
{
    /* 以下成员用户不用关注，也不能随便修改 */
    const char * _ObjProcName;
    struct rb_root _ObjRoot;
    unsigned int _ObjCount;
    unsigned int  _ObjTotalSize;
    struct proc_dir_entry * _ProcEntry;
    struct file_operations _FileOp;
    struct seq_operations _SeqOp;
     unsigned long _SpinFlag;             /* 中断锁标志 */
    spinlock_t _ObjLocker;          /* 先使用关软中断的锁，可以在定时器使用该机制 */
    void * _pLastObjShow;

    /* 每行记录打印的大小，当Seq文件缓存小于该值时，就分页输出，用户根据每个记录的输出大小来配置该值。默认128，设置过小就无法打印输出了。 */
    unsigned int MaxShowSize;
        
    /* 比较回调函数必需存在，必需返回大小关系，如果相等就表示是同一个对象，即内存地址都是一样的 */
    pf_hw_ssp_ker_obj_cmp ObjCmp;       /* 对象比较函数，用于比较两个对象的大小，需要返回大小关系 */

    /* 以下函数可以为空，如果为空，则对应的功能不支持 */
    pf_hw_ssp_ker_obj_show ObjShow;     /* 通过proc文件名读取对象信息时显示，当pData为空时，显示表头，如果为空就不支持proc读取 */
    pf_hw_ssp_ker_obj_ioctl ObjIoctl;   /* ioctl处理函数，如果为空则不支持ioctl */
    pf_hw_ssp_ker_obj_write ObjWrite;   /* 写函数支持，如果为空，则不支持写操作 */
    //
    pf_hw_ssp_ker_obj_search_cmp ObjSearcher;   /* 对象查询回调函数，如果使用默认的ioctl，必需实现该函数 */
    //
} hw_ssp_ker_obj_container;	
/****************************************************************************************************************/

#ifdef __cplusplus
}
#endif

#endif 
