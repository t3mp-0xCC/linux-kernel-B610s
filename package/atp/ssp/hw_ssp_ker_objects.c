

#include <linux/vmalloc.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <net/sock.h>
#include <net/netlink.h>
#include <linux/rbtree.h>
#include <linux/errno.h>
#include <linux/dcache.h>
#include <asm/bug.h>
//#include "atpconfig.h"
#include "hw_ssp_ker_objects.h"
#include <linux/proc_fs.h>

/*lint -save -e64 -e78 -e515 -e530*/

#define Region(s) (1)
#define PRIVATE static
#define MAX_PROC_PATH (64)
#define HW_RET_OS_ADDR_EXIST (-1)
#define HW_OS_ERR_NOT_INIT (-1)
#define HW_OS_RT_ARGC_ERROR (-1)
#define HW_RET_OS_NULL_PTR (NULL)
#define HW_RET_OS_ADDR_NOTEXIST (-1)
#define HW_SSP_KER_OBJ_DEFAULT_SHOW_SIZE (256)
#define HW_SSP_KOBJ_LOG(a,b,c,d,e) //proc_ker_last_word(__FILE__,__LINE__,a,b,c,d,e)

/* obj节点数据，pObj指向实际的数据，数据格式为针线式(即指针后面就是实际数据) */
typedef struct tagHW_SSP_KER_OBJ_NODE
{
    unsigned short ObjMagicFlag;       /* 用于标识这块内存是否是通过Obj机制申请的 */
    unsigned short ObjSize;
    struct rb_node node;
    void* pObj;
} HW_SSP_KER_OBJ_NODE;

/* obj的自管理对象，用于管理HW_SSP_KER_OBJ_CONTAINER */
typedef struct tagHW_SSP_KER_OBJ_MANAGER
{
    hw_ssp_ker_obj_container* pObjContainer;
} HW_SSP_KER_OBJ_MANAGER;

 void* hw_ssp_ker_obj_search(hw_ssp_ker_obj_container* pstContainer,
                                   pf_hw_ssp_ker_obj_search_cmp Searcher, int KeyData);


/* 唯一的全局变量，用于管理所有的obj对象 */
hw_ssp_ker_obj_container* g_ObjManager = NULL;

/* 内部查询函数申明，详细参见函数注释 */
PRIVATE int hw_ssp_ker_obj_manager_search(void* pObj, int KeyData);
extern int __init hw_ssp_ker_wq_init(void);
extern int __init hw_ssp_ker_last_word_init(void);
extern int hw_ssp_ker_panic_init(void);
void hw_ssp_ker_obj_dispose(hw_ssp_ker_obj_container* pstContainer);

/*获取字符串中目标字符的位置*/
int hw_ssp_ker_string_last_index_of(const char * pString, char Target)
{
    int last_index = -1;
    int index = 0;
    int count = 0;

    while (pString[index] != '\0' && count < 64)
    {
        if (pString[index] == Target)
        {
            last_index = index;
        }
        index ++;
	 count++;
    }

    return last_index;
}


struct proc_dir_entry* g_pstProcDir = NULL;
struct proc_dir_entry* hw_ssp_basic_get_atp_dir(void)
{
    if (g_pstProcDir == NULL)
    {
        /* create /proc/atp_proc */
        g_pstProcDir = proc_mkdir(HW_SSP_ATP_PROC, NULL);
        //g_pstProcDir->nlink = 0;
    }
    return g_pstProcDir;
}

#if  Region("红黑树管理")
/* 考虑应用场景，红黑树操作中不能记录LSW */

static void hw_ssp_ker_obj_lock(hw_ssp_ker_obj_container* pObjInfo)
{
    spin_lock_irqsave(&pObjInfo->_ObjLocker, pObjInfo->_SpinFlag);
    return;
}

/*****************************************************************************
 Function name: hw_ssp_ker_obj_unlock
 Description  : 解锁函数封装。
 Input        : hw_ssp_ker_obj_container * pObjInfo
 Output       : None
 Return Value : None

 History    :
    1.Date         : D2013_06_18
     Author       : wutiejun
     Modification : Created function

*****************************************************************************/
static void hw_ssp_ker_obj_unlock(hw_ssp_ker_obj_container* pObjInfo)
{
    spin_unlock_irqrestore(&pObjInfo->_ObjLocker, pObjInfo->_SpinFlag);
    return;
}

/*****************************************************************************
 Function name: hw_ssp_ker_obj_add_to_rbtree
 Description  : 将对象添加到容器中。容器使用红黑树管理。
 Input        : hw_ssp_ker_obj_container * pObjInfo  容器对象
                HW_SSP_KER_OBJ_NODE * pAddNode       需要添加的对象
 Output       : None
 Return Value :
    Successed : 0
    Failed    : 错误码

 History    :
    1.Date         : D2013_06_18
     Author       : wutiejun
     Modification : Created function

*****************************************************************************/
PRIVATE int hw_ssp_ker_obj_add_to_rbtree(hw_ssp_ker_obj_container* pObjInfo,
        HW_SSP_KER_OBJ_NODE* pAddNode)
{
    struct rb_node** new_node = NULL;
    struct rb_node* parent = NULL;
    HW_SSP_KER_OBJ_NODE* pstTmNode = NULL;
    int result = 0;

    hw_ssp_ker_obj_lock(pObjInfo);

    new_node = &(pObjInfo->_ObjRoot.rb_node);

    /* 红黑树遍历，找到对应的节点 */
    while (*new_node)
    {
        pstTmNode = container_of(*new_node, HW_SSP_KER_OBJ_NODE, node);
        parent = *new_node;

        result = pObjInfo->ObjCmp(pAddNode->pObj, pstTmNode->pObj);

        if (result < 0)
        {
            new_node = &((*new_node)->rb_left);
        }
        else if (result > 0)
        {
            new_node = &((*new_node)->rb_right);
        }
        else
        {
            /* 同一容器中，不能存在Key相同的两个对象，记录LSW，返回错误 */
            hw_ssp_ker_obj_unlock(pObjInfo);
            HW_SSP_KOBJ_LOG(HW_RET_OS_ADDR_EXIST, pAddNode->pObj, pstTmNode->pObj,
                            pAddNode, pObjInfo);
            return (int)HW_RET_OS_ADDR_EXIST;
        }
    }

    /* 将节点添加到红黑树并重新对红黑树上色，使之平衡 */
    rb_link_node(&pAddNode->node, parent, new_node);
    rb_insert_color(&pAddNode->node, &pObjInfo->_ObjRoot);

    pObjInfo->_ObjCount ++;
    pObjInfo->_ObjTotalSize += pAddNode->ObjSize;
    pAddNode->ObjMagicFlag = (unsigned short)(unsigned int)pObjInfo;

    hw_ssp_ker_obj_unlock(pObjInfo);

    return 0;

}

/*****************************************************************************
 Function name: hw_ssp_ker_obj_get_from_rbtree
 Description  : 从容器中取得取得的引用，使用对象时已经在锁外了，
                需要注意如果业务需要，还要再添加锁。
 Input        : hw_ssp_ker_obj_container * pObjContainer  容器
                pf_hw_ssp_ker_obj_search_cmp Searcher     查询的回调函数
                unsigned int KeyData                           查询的Key数据
 Output       : None
 Return Value :
    Successed : 对象的引用地址
    Failed    : NULL

 History    :
    1.Date         : D2013_06_18
     Author       : wutiejun
     Modification : Created function

*****************************************************************************/
PRIVATE HW_SSP_KER_OBJ_NODE* hw_ssp_ker_obj_get_from_rbtree(hw_ssp_ker_obj_container* pObjContainer,
        pf_hw_ssp_ker_obj_search_cmp Searcher,
        unsigned int KeyData)
{
    struct rb_node* pTempNode = NULL;
    HW_SSP_KER_OBJ_NODE* pFindObj = NULL;
    HW_SSP_KER_OBJ_NODE* pstObj = NULL;
    int result = 0;

    hw_ssp_ker_obj_lock(pObjContainer);

    pTempNode = pObjContainer->_ObjRoot.rb_node;

    while (pTempNode)
    {
        pstObj = container_of(pTempNode, HW_SSP_KER_OBJ_NODE, node);

        result = Searcher(pstObj->pObj, (int)KeyData);
        if (result < 0)
        {
            pTempNode = pTempNode->rb_left;
        }
        else if (result > 0)
        {
            pTempNode = pTempNode->rb_right;
        }
        else
        {
            pFindObj = pstObj;
            break;
        }
    }

    hw_ssp_ker_obj_unlock(pObjContainer);

    return pFindObj;

}

#endif //Region

#ifdef OBJ_SUOPORT_IOCTL /*暂未实现*/
typedef int (*pf_hw_ssp_ker_obj_ioctl_call)(hw_ssp_ker_obj_container* pContainer, HW_SSP_KER_OBJ_IOCTL* pstCtrl);
#if Region("支持IOCTL操作:")
/* ioctrl操作中可以使用lastword记录错误 */

PRIVATE int hw_ssp_ker_obj_ioctl_add(hw_ssp_ker_obj_container* pContainer,
                                     HW_SSP_KER_OBJ_IOCTL* pstCtrl)
{
    void* pObj = NULL;
    pObj = hw_ssp_ker_obj_new(pstCtrl->ObjSize, GFP_KERNEL);
    hw_ssp_ker_obj_add(pContainer, pObj);
    return 0;
}

PRIVATE int hw_ssp_ker_obj_ioctl_del(hw_ssp_ker_obj_container* pContainer,
                                     HW_SSP_KER_OBJ_IOCTL* pstCtrl)
{
    void* pObj = NULL;
    pObj = hw_ssp_ker_obj_search(pContainer, pContainer->ObjSearcher, pstCtrl->Key);
    hw_ssp_ker_obj_del(pContainer, pObj);
    hw_ssp_ker_obj_free(pObj);
    return 0;
}

PRIVATE int hw_ssp_ker_obj_ioctl_get(hw_ssp_ker_obj_container* pContainer,
                                     HW_SSP_KER_OBJ_IOCTL* pstCtrl)
{
    void* pObj = NULL;
    HW_SSP_KER_OBJ_NODE* pObjNode = NULL;
    int Ret = 0;

    pObj = hw_ssp_ker_obj_search(pContainer, pContainer->ObjSearcher, pstCtrl->Key);
    pObjNode = (HW_SSP_KER_OBJ_NODE*)((unsigned int)pObj - sizeof(HW_SSP_KER_OBJ_NODE));
    if (pObjNode->ObjSize > pstCtrl->ObjSize)
    {
        return -1;
    }

    Ret = copy_to_user(pstCtrl->pObj, pObj, pObjNode->ObjSize);
    if (0 != Ret)
    {
        return -1;
    }

    return 0;
}

PRIVATE int hw_ssp_ker_obj_ioctl_set(hw_ssp_ker_obj_container* pContainer,
                                     HW_SSP_KER_OBJ_IOCTL* pstCtrl)
{
    void* pObj = NULL;
    HW_SSP_KER_OBJ_NODE* pObjNode = NULL;
    int Ret = 0;

    pObj = hw_ssp_ker_obj_search(pContainer, pContainer->ObjSearcher, pstCtrl->Key);
    pObjNode = (HW_SSP_KER_OBJ_NODE*)((unsigned int)pObj - sizeof(HW_SSP_KER_OBJ_NODE));
    if (pObjNode->ObjSize > pstCtrl->ObjSize)
    {
        return -1;
    }

    Ret = copy_from_user(pObjNode->pObj, pstCtrl->pObj, pstCtrl->ObjSize);
    if (0 != Ret)
    {
        return -1;
    }

    return 0;

}

PRIVATE int hw_ssp_ker_obj_ioctl_get_first(hw_ssp_ker_obj_container* pContainer,
        HW_SSP_KER_OBJ_IOCTL* pstCtrl)
{
    void* pObj = NULL;
    HW_SSP_KER_OBJ_NODE* pObjNode = NULL;
    int Ret = 0;

    pObj = hw_ssp_ker_obj_first(pContainer);
    pObjNode = (HW_SSP_KER_OBJ_NODE*)((unsigned int)pObj - sizeof(HW_SSP_KER_OBJ_NODE));
    if (pObjNode->ObjSize > pstCtrl->ObjSize)
    {
        return -1;
    }

    Ret = copy_to_user(pstCtrl->pObj, pObj, pObjNode->ObjSize);
    if (0 != Ret)
    {
        return -1;
    }

    return 0;
}

PRIVATE int hw_ssp_ker_obj_ioctl_get_last(hw_ssp_ker_obj_container* pContainer,
        HW_SSP_KER_OBJ_IOCTL* pstCtrl)
{
    void* pObj = NULL;
    HW_SSP_KER_OBJ_NODE* pObjNode = NULL;
    int Ret = 0;

    pObj = hw_ssp_ker_obj_last(pContainer);
    pObjNode = (HW_SSP_KER_OBJ_NODE*)((unsigned int)pObj - sizeof(HW_SSP_KER_OBJ_NODE));
    if (pObjNode->ObjSize > pstCtrl->ObjSize)
    {
        return -1;
    }

    Ret = copy_to_user(pstCtrl->pObj, pObj, pObjNode->ObjSize);
    if (0 != Ret)
    {
        return -1;
    }

    return 0;
}

PRIVATE pf_hw_ssp_ker_obj_ioctl_call hw_ssp_ker_obj_get_ioctl_pf(unsigned int cmd)
{
    struct ioctl_pf
    {
        unsigned int cmd;
        pf_hw_ssp_ker_obj_ioctl_call pf;
    };

    /* 使用简单的表驱动来实现ioctl命令分发 */
    struct ioctl_pf all_ioctls[] =
    {
        {HW_SSP_OBJ_PROC_ADD, hw_ssp_ker_obj_ioctl_add},
        {HW_SSP_OBJ_PROC_DEL, hw_ssp_ker_obj_ioctl_add},
        {HW_SSP_OBJ_PROC_GET, hw_ssp_ker_obj_ioctl_add},
        {HW_SSP_OBJ_PROC_SET, hw_ssp_ker_obj_ioctl_add},
        {HW_SSP_OBJ_PROC_GET_FIRST, hw_ssp_ker_obj_ioctl_get_first},
        {HW_SSP_OBJ_PROC_GET_LAST, hw_ssp_ker_obj_ioctl_get_last},
    };

    pf_hw_ssp_ker_obj_ioctl_call pf_ioctl = NULL;
    int index = 0;

    for (index = 0; index <  sizeof(all_ioctls) / sizeof(struct ioctl_pf); index ++)
    {
        if (all_ioctls[index].cmd == cmd)
        {
            pf_ioctl = all_ioctls[index].pf;
            break;
        }
    }

    return pf_ioctl;

}

#endif //Region
#endif

#if Region("文件操作:")
/*****************************************************************************
 Function name: hw_ssp_ker_obj_get_container
 Description  : 通过打开的文件信息取得系统容器对象。
 Input        : struct file *pfile  指向proc目录下的文件对象
 Output       : None
 Return Value :
    Successed : 容器对象引用地址
    Failed    : NULL

 History    :
    1.Date         : D2013_06_18
     Author       : wutiejun
     Modification : Created function

*****************************************************************************/
PRIVATE hw_ssp_ker_obj_container* hw_ssp_ker_obj_get_container(struct file* pfile)
{
    HW_SSP_KER_OBJ_MANAGER* pObjManager = NULL;
    char* pFilePath = NULL;
    char* pFileName = NULL;
    char ProcName[MAX_PROC_PATH] = {0};
    int index = 0;

 #if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,30)
      pFilePath = d_path(pfile->f_dentry,pfile->f_vfsmnt,ProcName,MAX_PROC_PATH - 1);
 #else
     pFilePath = d_path(&pfile->f_path, ProcName, MAX_PROC_PATH - 1);
 #endif
    if (unlikely(NULL == pFilePath))
    {
        BUG();
        return NULL;
    }

    index = hw_ssp_ker_string_last_index_of(pFilePath, '/');
    pFileName = pFilePath + index + 1;
    pObjManager = (HW_SSP_KER_OBJ_MANAGER*)hw_ssp_ker_obj_search(g_ObjManager,
                  hw_ssp_ker_obj_manager_search, (int)pFileName);
    if (unlikely(NULL == pObjManager))
    {
        printk("Warning: pObjManager is NULL, File Path [%s]\n", pFilePath);
        return NULL;
    }

    return pObjManager->pObjContainer;

}

/*****************************************************************************
 Function name: hw_ssp_ker_obj_ioctl
 Description  : 根据查找到的容器回调不同的ioctl。
 Input        : struct inode *pinode  proc目录下的文件节点
                struct file *pfile    proc目录下的文件对象
                unsigned int cmd           ioctl操作的命令字
                unsigned long arg          ioctl操作的参数
 Output       : None
 Return Value :
    Successed : 0
    Failed    : 错误码

 History    :
    1.Date         : D2013_06_18
     Author       : wutiejun
     Modification : Created function

*****************************************************************************/
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36)
PRIVATE int hw_ssp_ker_obj_ioctl(struct inode * pInode, struct file * pFile, unsigned int cmd, unsigned long arg)
#else
PRIVATE int hw_ssp_ker_obj_ioctl(struct file *pFile,unsigned int cmd, unsigned long arg)
#endif
{
    hw_ssp_ker_obj_container* pContainer = NULL;
    int ret = 0;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36)		
	    preempt_disable();
#endif

    pContainer = hw_ssp_ker_obj_get_container(pFile);
    if (unlikely(NULL == pContainer))
    {
        HW_SSP_KOBJ_LOG(HW_OS_ERR_NOT_INIT, cmd, 0, 0, 0);
        return (int)HW_OS_ERR_NOT_INIT;
    }

    /* 内部已经支持的ioctl，就是支持数据的添加、删除、修改和查询 */


    if (unlikely(NULL == pContainer->ObjIoctl))
    {
        HW_SSP_KOBJ_LOG(HW_OS_ERR_NOT_INIT, cmd, pContainer, 0, 0);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36)		
	    preempt_enable();
#endif
		
        return (int)HW_OS_ERR_NOT_INIT;
    }

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36)
    ret = pContainer->ObjIoctl(pInode, pFile, cmd, arg);
#else
    ret = pContainer->ObjIoctl(pFile, cmd, arg);
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36)		
	    preempt_enable();
#endif
    return ret;
}

/*****************************************************************************
 Function name: hw_ssp_ker_obj_open
 Description  : 根据打开的不同文件，找到对应的容器，然后打开seq文件。
 Input        : struct inode *pinode    proc目录下的文件节点
                struct file *filp       proc目录下的文件对象
 Output       : None
 Return Value :
    Successed : 0
    Failed    : 错误码

 History    :
    1.Date         : D2013_06_18
     Author       : wutiejun
     Modification : Created function

*****************************************************************************/
PRIVATE int hw_ssp_ker_obj_open(struct inode* pinode, struct file* filp)
{
    hw_ssp_ker_obj_container* pContainer = NULL;
    struct seq_file* pSeq = NULL;
    int ret = 0;

    pContainer = hw_ssp_ker_obj_get_container(filp);
    if (unlikely(!pContainer))
    {
        HW_SSP_KOBJ_LOG(HW_RET_OS_ADDR_NOTEXIST, 0, 0, 0, 0);
        return HW_RET_OS_ADDR_NOTEXIST;
    }

    ret = seq_open(filp, &pContainer->_SeqOp);
    if (ret)
    {
        HW_SSP_KOBJ_LOG(ret, 0, 0, 0, 0);
        return ret;
    }

    /* 将seq文件的私有数据设置为我们需要使用的对象容器 */
    pSeq = (struct seq_file*)filp->private_data;
    if (NULL == pSeq)
    {
        HW_SSP_KOBJ_LOG(HW_RET_OS_ADDR_NOTEXIST, 0, 0, 0, 0);
        return (int)HW_RET_OS_ADDR_NOTEXIST;
    }

    pSeq->private = pContainer;

    return 0;

}

/*****************************************************************************
 Function name: hw_ssp_ker_obj_write
 Description  : proc目录下的文件写操作。
 Input        : struct file * file              proc目录下的文件对象
                const char __user * acBuffer    用户写入的数据缓存区，指向的是用户空间
                size_t buflen                   写入的缓存区大小
                loff_t * fpos                   写入文件的偏移
 Output       : None
 Return Value :
    Successed : 写入文件的数据大小
    Failed    : 写入文件的数据大小，一般为0表示写入失败

 History    :
    1.Date         : D2013_09_24
     Author       : wutiejun
     Modification : Created function

*****************************************************************************/
PRIVATE ssize_t hw_ssp_ker_obj_write(struct file* file, const char __user* acBuffer,
                                     size_t buflen, loff_t* fpos)
{
    hw_ssp_ker_obj_container* pContainer = NULL;
    int ret = 0;

    /* 如果对象不存在，或者没有挂回写函数，都直接返回0，表示不支持写操作 */
    pContainer = hw_ssp_ker_obj_get_container(file);
    if (unlikely(NULL == pContainer))
    {
        return 0;
    }

    if (NULL == pContainer->ObjWrite)
    {
        return 0;
    }

    /* 正确回调写操作函数，需要返回写入的字节数 */
    ret = pContainer->ObjWrite(file, acBuffer, buflen, fpos);

    return ret;

}

/*****************************************************************************
 Function name: hw_ssp_ker_obj_seq_start
 Description  : seq文件系统的开始读取。
 Input        : struct seq_file *m      seq文件对象
                loff_t *pos             文件偏移
 Output       : None
 Return Value :
    Successed : 容器内对象的引用地址，或者起始标志SEQ_START_TOKEN
    Failed    : NULL，表示容器内没有对象

 History    :
    1.Date         : D2013_06_18
     Author       : wutiejun
     Modification : Created function

*****************************************************************************/
PRIVATE void* hw_ssp_ker_obj_seq_start(struct seq_file* m, loff_t* pos)
{
    hw_ssp_ker_obj_container* pContainer = NULL;
    pContainer = (hw_ssp_ker_obj_container*)m->private;

    /* 内核会保证调用hw_ssp_ker_obj_seq_start后，一定调用hw_ssp_ker_obj_seq_stop，可以解锁 */
    hw_ssp_ker_obj_lock(pContainer);

    /* 如果没有挂show函数，则直接返回 */
    if (NULL == pContainer->ObjShow)
    {
        return NULL;
    }

    /* 第一次开始读取时pos为0 */
    if (*pos <= 0)
    {
        return SEQ_START_TOKEN;
    }
    else
    {
        return pContainer->_pLastObjShow;
    }

}

/*****************************************************************************
 Function name: hw_ssp_ker_obj_seq_stop
 Description  : seq文件系统的停止。
 Input        : struct seq_file *m  seq文件对象
                void *v          上次读取的容器中的对象，一般不使用
 Output       : None
 Return Value : None

 History    :
    1.Date         : D2013_06_18
     Author       : wutiejun
     Modification : Created function

*****************************************************************************/
PRIVATE void hw_ssp_ker_obj_seq_stop(struct seq_file* m, void* v)
{
    hw_ssp_ker_obj_container* pContainer = NULL;

    pContainer = (hw_ssp_ker_obj_container*)m->private;
    hw_ssp_ker_obj_unlock(pContainer);

    return;
}

/*****************************************************************************
 Function name: hw_ssp_ker_obj_seq_next
 Description  : seq文件的下一个读取操作。
 Input        : struct seq_file *m      seq文件对象
                void *v              容器中的当前对象
                loff_t *pos             文件偏移
 Output       : None
 Return Value :
    Successed : 容器中的下一个对象
    Failed    : NULL，表示容器中已经没有对象

 History    :
    1.Date         : D2013_06_18
     Author       : wutiejun
     Modification : Created function

*****************************************************************************/
PRIVATE void* hw_ssp_ker_obj_seq_next(struct seq_file* m, void* v, loff_t* pos)
{
    hw_ssp_ker_obj_container* pContainer = NULL;
    struct rb_node* next_node = NULL;
    HW_SSP_KER_OBJ_NODE* pShowObj = NULL;

    pContainer = (hw_ssp_ker_obj_container*)m->private;

    if (SEQ_START_TOKEN == v)
    {
        next_node = rb_first(&pContainer->_ObjRoot);
        if (next_node != NULL)
        {
            pShowObj = rb_entry(next_node, HW_SSP_KER_OBJ_NODE, node);
            pContainer->_pLastObjShow = pShowObj;
        }
        return pShowObj;
    }

    pShowObj = (HW_SSP_KER_OBJ_NODE*)v;
    if (NULL == pShowObj)
    {
        return pShowObj;
    }

    next_node = rb_next(&pShowObj->node);
    if (next_node != NULL)
    {
        pShowObj = rb_entry(next_node, HW_SSP_KER_OBJ_NODE, node);
    }
    else
    {
        pShowObj = NULL;
    }

    pContainer->_pLastObjShow = pShowObj;

    return pShowObj;

}

/*****************************************************************************
 Function name: hw_ssp_ker_obj_seq_show
 Description  : Seq文件的show操作，通过private字段取得容器，并回调相关的show
                操作。
 Input        : struct seq_file *m      seq文件对象
                void *v              当前读取的容器中的对象
 Output       : None
 Return Value :
    Successed : 0
    Failed    : 非0，表示输出失败，下次读取进继续从当前对象输出

 History    :
    1.Date         : D2013_06_18
     Author       : wutiejun
     Modification : Created function

*****************************************************************************/
PRIVATE int hw_ssp_ker_obj_seq_show(struct seq_file* m, void* v)
{
    hw_ssp_ker_obj_container* pContainer = NULL;
    HW_SSP_KER_OBJ_NODE* pObjNode = NULL;

    pContainer = (hw_ssp_ker_obj_container*)m->private;

    if ((0 == v) || (0 == pContainer->ObjShow))
    {
        /* 如果读取的对象为空，或者没有挂show函数，直接返回，不输出 */
        return 0;
    }

    if (SEQ_START_TOKEN == v)
    {
        /* 如果是初始标志，使用NULL参数调用输出，表示需要输出表头数据 */
        pContainer->ObjShow(m, NULL, 0);
        return 0;
    }

    pObjNode = (HW_SSP_KER_OBJ_NODE*)v;

    /* 如果打印缓存大于最大缓存就打印输出，否则暂时不输出，等下一次输出 */
    if (m->size - m->count > pContainer->MaxShowSize)
    {
        return pContainer->ObjShow(m, pObjNode->pObj, pObjNode->ObjSize);
    }
    else
    {
        return -1;
    }
}

/******************************************************************************
proc目录下对象的文件操作结构，所有的文件操作函数都挂在这里。
这里是最少的需要实现的回调函数，文件操作的其它函数不用实现。
*******************************************************************************/
struct file_operations hw_ssp_ker_obj_proc_op =
{
    .owner = THIS_MODULE,
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36)	
    .ioctl  = hw_ssp_ker_obj_ioctl,     /* ioctl操作回调函数 */
#else
    .unlocked_ioctl  = hw_ssp_ker_obj_ioctl,     /* ioctl操作回调函数 */
#endif
    .open = hw_ssp_ker_obj_open,        /* open操作回调函数 */
    .write = hw_ssp_ker_obj_write,      /* write操作回调函数 */
    .read = seq_read,                   /* read操作回调函数 */
    .release = seq_release,             /* release操作回调函数 */
    .llseek = seq_lseek,                /* llseek操作回调函数 */
};

#endif //Region

#if  Region("对外API")
/* 考虑应用场景，内核API中不能记录LSW */
/*****************************************************************************
 Function name: hw_ssp_ker_obj_new
 Description  : 创建一个可以添加到容器中的对象。
 Input        : int ObjSize      对象的大小
                int Flag         内核申请内存的Flag，用于标志申请内存时是否可以阻塞
 Output       : None
 Return Value :
    Successed : 对象的内存地址
    Failed    : NULL，申请内存失败

 History    :
    1.Date         : D2013_06_18
     Author       : wutiejun
     Modification : Created function

*****************************************************************************/
 void* hw_ssp_ker_obj_new(int ObjSize, int Flag)
{
    unsigned int TotalSize = 0;
    HW_SSP_KER_OBJ_NODE* pObjNode = NULL;

    if (unlikely(ObjSize <= 0))
    {
        HW_SSP_KOBJ_LOG(HW_OS_RT_ARGC_ERROR, ObjSize, 0, 0, 0);
        return NULL;
    }

    TotalSize = (unsigned int)ObjSize + sizeof(HW_SSP_KER_OBJ_NODE);

    pObjNode = kmalloc(TotalSize, (unsigned int)Flag);
    if (unlikely(NULL == pObjNode))
    {
        /* 如果无法申请到内存，根据SE要求，直接panic复位，由其它机制分析定位内存问题 */
        panic("hw_ssp_ker_obj_new:out of memory!:size=%d;flag=%d;\r\n", TotalSize, Flag);
        return NULL;
    }

    memset(pObjNode, 0, TotalSize);
    pObjNode->ObjSize = (unsigned short)TotalSize;  /* 这里仅记录下方便分析定位，使用ushort没有问题 */
    pObjNode->pObj = (void*)((unsigned int)pObjNode + sizeof(HW_SSP_KER_OBJ_NODE));

    return pObjNode->pObj;

}

/*****************************************************************************
 Function name: hw_ssp_ker_obj_free
 Description  : 释放一个对象，当添加到容器中失败，或者不再使用该对象时，需要
                释放。如果业务需要，还要再上业务锁。
 Input        : void * pObj      使用hw_ssp_ker_obj_new生成的对象
 Output       : None
 Return Value : None

 History    :
    1.Date         : D2013_06_18
     Author       : wutiejun
     Modification : Created function

*****************************************************************************/
void hw_ssp_ker_obj_free(void* pObj)
{
    HW_SSP_KER_OBJ_NODE* pObjNode = NULL;

    pObjNode = (HW_SSP_KER_OBJ_NODE*)((unsigned int)pObj - sizeof(HW_SSP_KER_OBJ_NODE));
    if (unlikely(pObjNode->ObjMagicFlag != 0))
    {
        /* 该对象还中某个容器中，不能free对象，需要先从容器中删除以后再释放 */
        HW_SSP_KOBJ_LOG(HW_OS_RT_ARGC_ERROR, pObj, pObjNode->ObjMagicFlag, 0, 0);
        return;
    }

    /* free前设置为0，如果业务释放后还有使用内存，就会因为空指针异常，可以提前暴露问题 */
    memset(pObjNode, 0, sizeof(HW_SSP_KER_OBJ_NODE));

    kfree(pObjNode);

    return;

}

/*****************************************************************************
 Function name: hw_ssp_ker_obj_add
 Description  : 将对象添加到指定容器中。
 Input        : hw_ssp_ker_obj_container * pObjContainer    容器对象
                void * pObj                              需要添加到容器中的对象
 Output       : None
 Return Value :
    Successed : 0
    Failed    : 错误码

 History    :
    1.Date         : D2013_06_18
     Author       : wutiejun
     Modification : Created function

*****************************************************************************/
int hw_ssp_ker_obj_add(hw_ssp_ker_obj_container* pObjContainer, void* pObj)
{
    HW_SSP_KER_OBJ_NODE* pObjNode = NULL;

    if (unlikely( (NULL == pObjContainer) || (NULL == pObjContainer->ObjCmp) ))
    {
        HW_SSP_KOBJ_LOG(HW_OS_RT_ARGC_ERROR, pObjContainer, 0, 0, 0);
        return (int)HW_OS_RT_ARGC_ERROR;
    }

    pObjNode = (HW_SSP_KER_OBJ_NODE*)((unsigned int)pObj - sizeof(HW_SSP_KER_OBJ_NODE));
    if (unlikely(pObjNode->ObjMagicFlag != 0))
    {
        /* 已经在某个容器中了，不能重复添加 */
        HW_SSP_KOBJ_LOG(HW_OS_RT_ARGC_ERROR, pObjContainer, pObjNode->ObjMagicFlag, 0, 0);
        return (int)HW_OS_RT_ARGC_ERROR;
    }

    return hw_ssp_ker_obj_add_to_rbtree(pObjContainer,  pObjNode);
}

/*****************************************************************************
 Function name: hw_ssp_ker_obj_del
 Description  : 从指定容器上删除对象。
 Input        : hw_ssp_ker_obj_container * pObjContainer  容器对象
                void * pObj                            需要删除的对象
 Output       : None
 Return Value : None

 History    :
    1.Date         : D2013_06_18
     Author       : wutiejun
     Modification : Created function

*****************************************************************************/
PRIVATE void hw_ssp_ker_obj_del_inner(hw_ssp_ker_obj_container* pObjContainer,
                                      HW_SSP_KER_OBJ_NODE* pObjNode)
{
    rb_erase(&pObjNode->node, &pObjContainer->_ObjRoot);
    pObjContainer->_ObjTotalSize -= pObjNode->ObjSize;
    pObjNode->ObjMagicFlag = 0;
    pObjContainer->_ObjCount --;

    return;
}

/*****************************************************************************
 Function name: hw_ssp_ker_obj_del
 Description  : 从指定容器上删除对象。
 Input        : hw_ssp_ker_obj_container * pObjContainer  容器对象
                void * pObj                            需要删除的对象
 Output       : None
 Return Value : None

 History    :
    1.Date         : D2013_06_18
     Author       : wutiejun
     Modification : Created function

*****************************************************************************/
void hw_ssp_ker_obj_del(hw_ssp_ker_obj_container* pObjContainer, void* pObj)
{
    HW_SSP_KER_OBJ_NODE* pObjNode = NULL;

    pObjNode = (HW_SSP_KER_OBJ_NODE*)((unsigned int)pObj - sizeof(HW_SSP_KER_OBJ_NODE));
    if (unlikely(pObjNode->ObjMagicFlag != (unsigned short)(unsigned int)pObjContainer))
    {
        /* 对象不属于本容器 */
        HW_SSP_KOBJ_LOG(HW_OS_RT_ARGC_ERROR, pObjContainer, pObjNode->ObjMagicFlag, 0, 0);
        return ;
    }

    hw_ssp_ker_obj_lock(pObjContainer);
    hw_ssp_ker_obj_del_inner(pObjContainer, pObjNode);
    hw_ssp_ker_obj_unlock(pObjContainer);

    return;
}

/*****************************************************************************
 Function name: hw_ssp_ker_obj_search
 Description  : 在指定容器上通过KEY值查找对象，需要回调对象的search函数，如
                果没有指定search函数，则返回NULL。使用的是二分查找。
                因此Searcher必需返回正确的大小关系，否则无法查询。
 Input        : hw_ssp_ker_obj_container * pstContainer     容器对象
                pf_hw_ssp_ker_obj_search_cmp Searcher       查询函数
                unsigned int KeyData                             对象的key值
 Output       : None
 Return Value :
    Successed : 查询到的对象引用地址
    Failed    : NULL

 History    :
    1.Date         : D2013_06_18
     Author       : wutiejun
     Modification : Created function

*****************************************************************************/
void* hw_ssp_ker_obj_search(hw_ssp_ker_obj_container* pstContainer,
                                   pf_hw_ssp_ker_obj_search_cmp Searcher, int KeyData)
{
    HW_SSP_KER_OBJ_NODE* pObjNode = NULL;

    if (unlikely((NULL == pstContainer) || (NULL == Searcher)))
    {
        HW_SSP_KOBJ_LOG(HW_RET_OS_NULL_PTR, pstContainer, Searcher, 0, 0);
        return NULL;
    }

    pObjNode = hw_ssp_ker_obj_get_from_rbtree(pstContainer, Searcher, (unsigned int)KeyData);
    if (unlikely(NULL == pObjNode))
    {
        return pObjNode;
    }

    return pObjNode->pObj;
}

/*****************************************************************************
 Function name: hw_ssp_ker_obj_find
 Description  : 使用遍历的方式查询容器里的所有对象。
                Finder只用判断是否为找到的对象即可，如果是找到的对象，返回非0，否则返回0
 Input        : hw_ssp_ker_obj_container * pstContainer     容器对象
                pf_hw_ssp_ker_obj_search where              查询回调函数
                void * pData                             查询参数
 Output       : None
 Return Value :
    Successed : 对象的引用地址
    Failed    : NULL

 History    :
    1.Date         : D2013_06_18
     Author       : wutiejun
     Modification : Created function

*****************************************************************************/
void* hw_ssp_ker_obj_find(hw_ssp_ker_obj_container* pstContainer,
                                 pf_hw_ssp_ker_obj_find Finder, void* pData)
{
    void* FindObj = NULL;
    HW_SSP_KER_OBJ_NODE* pObjNode = NULL;
    struct rb_node* next_node = NULL;

    if (unlikely((NULL == pstContainer) || (NULL == Finder)))
    {
        HW_SSP_KOBJ_LOG(HW_RET_OS_NULL_PTR, pstContainer, Finder, 0, 0);
        return NULL;
    }

    hw_ssp_ker_obj_lock(pstContainer);

    next_node = rb_first(&pstContainer->_ObjRoot);
    while (next_node)
    {
        pObjNode = rb_entry(next_node, HW_SSP_KER_OBJ_NODE, node);
        if (Finder(pObjNode->pObj, pData))
        {
            FindObj = pObjNode->pObj;
            break;
        }
        next_node = rb_next(next_node);
    }

    hw_ssp_ker_obj_unlock(pstContainer);

    return FindObj;

}


/*****************************************************************************
 Function name: hw_ssp_ker_obj_all_del_call_back
 Description  : 用于对象全部删除的回调API。
 Input        : void * pObj
                void * pData
 Output       : None
 Return Value :
    Successed :
    Failed    :

 History    :
    1.Date         : D2013_11_05
     Author       : wutiejun
     Modification : Created function

*****************************************************************************/
int hw_ssp_ker_obj_all_del_call_back(void* pObj, void* pData)
{
    return 1;
}

/*****************************************************************************
 Function name: hw_ssp_ker_obj_foreach_del
 Description  : 删除所有满足Finder条件的对象，每删除一个时都会从新开始遍历，
                效率比较低。不建议使用，而应该使用指定对象的删除函数。
 Input        : hw_ssp_ker_obj_container * pstContainer     容器对象
                pf_hw_ssp_ker_obj_find Finder               删除时的比较查询函数，
                                                            当该函数返回非0时，表示删除查询到的对象。
                void * pData                             查询参数
 Output       : None
 Return Value : None

 History    :
    1.Date         : D2013_06_18
     Author       : wutiejun
     Modification : Created function

*****************************************************************************/
void hw_ssp_ker_obj_foreach_del(hw_ssp_ker_obj_container* pstContainer,
                                       pf_hw_ssp_ker_obj_find Finder, void* pData)
{
    HW_SSP_KER_OBJ_NODE* pObjNode = NULL;
    struct rb_node* next_node = NULL;

    if (unlikely((NULL == pstContainer) || (NULL == Finder)))
    {
        HW_SSP_KOBJ_LOG(HW_RET_OS_NULL_PTR, pstContainer, Finder, 0, 0);
        return ;
    }

    hw_ssp_ker_obj_lock(pstContainer);

    next_node = rb_first(&pstContainer->_ObjRoot);
    while (next_node)
    {
        pObjNode = rb_entry(next_node, HW_SSP_KER_OBJ_NODE, node);
        if (Finder(pObjNode->pObj, pData))
        {
            /* 删除当前节点 */
            hw_ssp_ker_obj_del_inner(pstContainer, pObjNode);
            hw_ssp_ker_obj_free(pObjNode->pObj);

            /* 删除以后，红黑树变化，需要重新从根节点遍历 */
            next_node = rb_first(&pstContainer->_ObjRoot);
        }
        else
        {
            next_node = rb_next(next_node);
        }
    }

    hw_ssp_ker_obj_unlock(pstContainer);

    return ;

}

/*****************************************************************************
 Function name: hw_ssp_ker_obj_foreach
 Description  : 循环遍历容器里的每一个对象，不能在回调函数中再操作容器，否则
                死锁。
 Input        : hw_ssp_ker_obj_container * pstContainer  容器对象
                pf_hw_ssp_ker_obj_find ForeachCallBack   遍历回调函数
                void * pData                          回调参数
 Output       : None
 Return Value : None

 History    :
    1.Date         : D2013_06_21
     Author       : wutiejun
     Modification : Created function

*****************************************************************************/
void hw_ssp_ker_obj_foreach(hw_ssp_ker_obj_container* pstContainer,
                                   pf_hw_ssp_ker_obj_find ForeachCallBack, void* pData)
{

    HW_SSP_KER_OBJ_NODE* pObjNode = NULL;
    struct rb_node* next_node = NULL;

    if (unlikely((NULL == pstContainer) || (NULL == ForeachCallBack)))
    {
        HW_SSP_KOBJ_LOG(HW_RET_OS_NULL_PTR, pstContainer, ForeachCallBack, 0, 0);
        return ;
    }

    hw_ssp_ker_obj_lock(pstContainer);

    next_node = rb_first(&pstContainer->_ObjRoot);
    while (next_node)
    {
        pObjNode = rb_entry(next_node, HW_SSP_KER_OBJ_NODE, node);
        if (ForeachCallBack(pObjNode->pObj, pData))
        {
            break;
        }
        next_node = rb_next(next_node);
    }

    hw_ssp_ker_obj_unlock(pstContainer);

    return ;

}

/*****************************************************************************
 Function name: hw_ssp_ker_obj_create_inner
 Description  : 管理容器的初始化。
 Input        : const char * _ObjProcName    容器对象名
 Output       : None
 Return Value :
    Successed : 创建的容器对象
    Failed    : NULL

 History    :
    1.Date         : D2013_06_18
     Author       : wutiejun
     Modification : Created function

*****************************************************************************/
PRIVATE hw_ssp_ker_obj_container* hw_ssp_ker_obj_create_inner(const char* ObjProcName)
{
    hw_ssp_ker_obj_container* pstObjInfo = NULL;
    struct proc_dir_entry* pTempProc = 0;
    struct proc_dir_entry* pAtpProc = 0;

    pAtpProc = hw_ssp_basic_get_atp_dir();
    //pTempProc = create_proc_entry(ObjProcName, 0, pAtpProc);
    pTempProc = proc_create(ObjProcName, 0, pAtpProc, &hw_ssp_ker_obj_proc_op);
    if (unlikely(NULL == pTempProc))
    {
        /* 创建对象失败，一般是对象名重复 */
        HW_SSP_KOBJ_LOG(HW_OS_RT_ARGC_ERROR, 0, 0, 0, 0);
        return NULL;
    }

    /* 申请内存并初始化结构 */
    pstObjInfo = kmalloc(sizeof(hw_ssp_ker_obj_container), GFP_KERNEL);
    if (unlikely(NULL == pstObjInfo))
    {
        /* 创建对象失败，系统无法运行，直接panic */
        panic("hw_ssp_ker_obj_create_inner:out of memory!:size=%d;flag=%d;\r\n",
              sizeof(hw_ssp_ker_obj_container), GFP_KERNEL);
        return NULL;
    }

    memset(pstObjInfo, 0, sizeof(hw_ssp_ker_obj_container));

    spin_lock_init(&pstObjInfo->_ObjLocker);
    pstObjInfo->_ObjProcName = ObjProcName;
    pstObjInfo->_ProcEntry = pTempProc;
    //
    //del this proc_create 最后一个参数会替代这里的赋值
    //pstObjInfo->_ProcEntry->proc_fops = &hw_ssp_ker_obj_proc_op;
    pstObjInfo->_SeqOp.start = hw_ssp_ker_obj_seq_start;
    pstObjInfo->_SeqOp.stop = hw_ssp_ker_obj_seq_stop;
    pstObjInfo->_SeqOp.next = hw_ssp_ker_obj_seq_next;
    pstObjInfo->_SeqOp.show = hw_ssp_ker_obj_seq_show;
    pstObjInfo->MaxShowSize = HW_SSP_KER_OBJ_DEFAULT_SHOW_SIZE;
    //
    return pstObjInfo;

}

/*****************************************************************************
 Function name: hw_ssp_ker_obj_create
 Description  : 对外API，用于创建容器对象，同时将容器对象当成obj-manager对象
                再添加到全局的管理容器中
 Input        : const char * ObjProcName  对象名
 Output       : None
 Return Value :
    Successed : 创建的容器对象
    Failed    : NULL

 History    :
    1.Date         : D2013_06_18
     Author       : wutiejun
     Modification : Created function

*****************************************************************************/
hw_ssp_ker_obj_container* hw_ssp_ker_obj_create(const char* ObjProcName)
{
    int ret = 0;
    hw_ssp_ker_obj_container* pstObjContainer = NULL;
    HW_SSP_KER_OBJ_MANAGER* pObjManager = NULL;

    pstObjContainer = hw_ssp_ker_obj_create_inner(ObjProcName);
    if (NULL == pstObjContainer)
    {
        HW_SSP_KOBJ_LOG(HW_OS_RT_ARGC_ERROR, 0, 0, 0, 0);
        return NULL;
    }

    /* 对外创建的对象都添加到全局的obj容器中 */
    pObjManager = (HW_SSP_KER_OBJ_MANAGER*)hw_ssp_ker_obj_new(sizeof(HW_SSP_KER_OBJ_MANAGER), GFP_KERNEL);
    pObjManager->pObjContainer = pstObjContainer;

    /* 将容器再做为对象添加到全局的容器中，形成合成的管理模式 */
    ret = hw_ssp_ker_obj_add(g_ObjManager, pObjManager);
    if (unlikely(ret != 0))
    {
        hw_ssp_ker_obj_free(pObjManager);
        hw_ssp_ker_obj_dispose(pstObjContainer);
        HW_SSP_KOBJ_LOG(ret, 0, 0, 0, 0);
        return NULL;
    }

    /* 返回创建的容器 */
    return pstObjContainer;

}

/*****************************************************************************
 Function name: 根据容器地址查询容器管理者。
 Description  :
 Input        : None
 Output       : None
 Return Value :
    Successed :
    Failed    :

 History    :
    1.Date         : D2013_11_21
     Author       : wutiejun
     Modification : Created function

*****************************************************************************/
PRIVATE int hw_ssp_ker_obj_dispose_finder(void* pObj, void* pData)
{
    HW_SSP_KER_OBJ_MANAGER* pManager = (HW_SSP_KER_OBJ_MANAGER*)pObj;
    return pManager->pObjContainer == pData;
}

/*****************************************************************************
 Function name: hw_ssp_ker_obj_dispose
 Description  : 释放整个容器。
 Input        : hw_ssp_ker_obj_container * pstContainer
                此时pstContainer其实就是g_ObjManager的一个对象，可以当成对象使用。
 Output       : None
 Return Value : None

 History    :
    1.Date         : D2013_06_18
     Author       : wutiejun
     Modification : Created function

*****************************************************************************/
void hw_ssp_ker_obj_dispose(hw_ssp_ker_obj_container* pstContainer)
{
    HW_SSP_KER_OBJ_NODE* pObjNode = NULL;
    struct rb_node* next_node = NULL;
    struct proc_dir_entry* pAtpProc = 0;
    HW_SSP_KER_OBJ_MANAGER* pManager = NULL;

    if (unlikely(NULL == pstContainer))
    {
        return;
    }

    /* 通过容器址址找到容器管理者 */
    pManager = hw_ssp_ker_obj_find(g_ObjManager, hw_ssp_ker_obj_dispose_finder, pstContainer);
    if (unlikely(NULL == pManager))
    {
        HW_SSP_KOBJ_LOG(HW_OS_RT_ARGC_ERROR, pstContainer, 0, 0, 0);
        return;
    }

    /* 先将容器从管理容器中删除 */
    hw_ssp_ker_obj_del(g_ObjManager, pManager);

    hw_ssp_ker_obj_lock(g_ObjManager);

    /* 再遍历删除该容器的所有对象 */
    next_node = rb_first(&pstContainer->_ObjRoot);
    while (next_node)
    {
        pObjNode = rb_entry(next_node, HW_SSP_KER_OBJ_NODE, node);
        hw_ssp_ker_obj_del_inner(pstContainer, pObjNode);
        hw_ssp_ker_obj_free(pObjNode->pObj);

        /* 每次都将第一个节点删除，这样节点引用不会乱 */
        next_node = rb_first(&pstContainer->_ObjRoot);
    }

    /* 删除proc文件 */
    pAtpProc = hw_ssp_basic_get_atp_dir();
    remove_proc_entry(pstContainer->_ObjProcName, pAtpProc);

    hw_ssp_ker_obj_unlock(g_ObjManager);

    /* 释放内存，设置容器指针为空 */
    hw_ssp_ker_obj_free(pManager);

    return;

}

/*****************************************************************************
 Function name: hw_ssp_ker_obj_first
 Description  : 取得排在第一个的元素。可以很容易的实现队列。
 Input        : hw_ssp_ker_obj_container * pstContainer
 Output       : None
 Return Value :
    Successed : 对象的引用地址
    Failed    : NULL

 History    :
    1.Date         : D2013_11_19
     Author       : wutiejun
     Modification : Created function

*****************************************************************************/
void* hw_ssp_ker_obj_first(hw_ssp_ker_obj_container* pstContainer)
{
    HW_SSP_KER_OBJ_NODE* pObjNode = NULL;
    struct rb_node* next_node = NULL;

    if (unlikely(NULL == pstContainer))
    {
        HW_SSP_KOBJ_LOG(HW_RET_OS_NULL_PTR, pstContainer, 0, 0, 0);
        return NULL;
    }

    hw_ssp_ker_obj_lock(pstContainer);
    next_node = rb_first(&pstContainer->_ObjRoot);
    hw_ssp_ker_obj_unlock(pstContainer);

    if (NULL == next_node)
    {
        return NULL;
    }

    pObjNode = rb_entry(next_node, HW_SSP_KER_OBJ_NODE, node);
    return pObjNode->pObj;


}

/*****************************************************************************
 Function name: hw_ssp_ker_obj_last
 Description  : 取得最后一个元素。可以很容易的实现栈。
 Input        : hw_ssp_ker_obj_container * pstContainer
 Output       : None
 Return Value :
    Successed : 对象的引用地址
    Failed    : NULL

 History    :
    1.Date         : D2013_11_19
     Author       : wutiejun
     Modification : Created function

*****************************************************************************/
void* hw_ssp_ker_obj_last(hw_ssp_ker_obj_container* pstContainer)
{
    HW_SSP_KER_OBJ_NODE* pObjNode = NULL;
    struct rb_node* next_node = NULL;

    if (unlikely(NULL == pstContainer))
    {
        HW_SSP_KOBJ_LOG(HW_RET_OS_NULL_PTR, pstContainer, 0, 0, 0);
        return NULL;
    }

    hw_ssp_ker_obj_lock(pstContainer);
    next_node = rb_last(&pstContainer->_ObjRoot);
    hw_ssp_ker_obj_unlock(pstContainer);

    if (NULL == next_node)
    {
        return NULL;
    }

    pObjNode = rb_entry(next_node, HW_SSP_KER_OBJ_NODE, node);
    return pObjNode->pObj;

}
#endif //Region

#if  Region("容器对象管理")
/*****************************************************************************
 Function name: hw_ssp_ker_obj_manager_cmp
 Description  : obj管理容器的比较函数，比较两个对象的大小。
 Input        : void * pObjA     对象A
                void * pObjB     对象B
 Output       : None
 Return Value : A和B两个对象的大小关系，如果A大于B则返回正数，等于返回0，否则返回负数

 History    :
    1.Date         : D2013_06_18
     Author       : wutiejun
     Modification : Created function

*****************************************************************************/
PRIVATE int hw_ssp_ker_obj_manager_cmp(void* pObjA, void* pObjB)
{
    HW_SSP_KER_OBJ_MANAGER* pA = (HW_SSP_KER_OBJ_MANAGER*)pObjA;
    HW_SSP_KER_OBJ_MANAGER* pB = (HW_SSP_KER_OBJ_MANAGER*)pObjB;
    return strcmp(pA->pObjContainer->_ObjProcName, pB->pObjContainer->_ObjProcName);
}

/*****************************************************************************
 Function name: hw_ssp_ker_obj_manager_search
 Description  : 管理容器的查询函数。用于二分查询的回调。
 Input        : void * pObj      正在查询的对象
                int KeyData      查询Key参数
 Output       : None
 Return Value : 查询对象与Key的大小关系，如果对象大于key，则返回正数，
                相等返回0，否则返回负数。

 History    :
    1.Date         : D2013_06_18
     Author       : wutiejun
     Modification : Created function

*****************************************************************************/
PRIVATE int hw_ssp_ker_obj_manager_search(void* pObj, int KeyData)
{
    HW_SSP_KER_OBJ_MANAGER* pObjManger = (HW_SSP_KER_OBJ_MANAGER*)pObj;
    const char* pObjManagerName = (const char*)KeyData;
    return strcmp(pObjManagerName, pObjManger->pObjContainer->_ObjProcName);
}

/*****************************************************************************
 Function name: hw_ssp_ker_obj_manager_show
 Description  : 管理容器的show
 Input        : struct seq_file *m      seq文件对象
                void * pData         需要show的对象
                unsigned int Size            对象的大小
 Output       : None
 Return Value :
    Successed : 0
    Failed    : 非0，表示该对象show失败，下次继续show该对象

 History    :
    1.Date         : D2013_06_18
     Author       : wutiejun
     Modification : Created function

*****************************************************************************/
PRIVATE int hw_ssp_ker_obj_manager_show(struct seq_file* m, void* pData, unsigned int Size)
{
    HW_SSP_KER_OBJ_MANAGER* pMyObj = pData;

    if (unlikely(NULL == pMyObj))
    {
        /* 输出Title */
        seq_printf(m, "Obj-Num  Total-Size Address    Obj-Name \r\n");
        return 0;
    }
    else
    {
        /* 输出内容 */
        hw_ssp_ker_obj_container* pObjContainer = pMyObj->pObjContainer;

        return seq_printf(m, "%-8d %-10d 0x%08x %s\r\n",
                          pObjContainer->_ObjCount,
                          pObjContainer->_ObjTotalSize,
                          (unsigned int)pObjContainer,
                          pObjContainer->_ObjProcName);
    }
}

/*****************************************************************************
 Function name: hw_ssp_ker_obj_init
 Description  : 初始化objects管理容器，并把自己添加到管理容器中
 Input        : void
 Output       : None
 Return Value :
    Successed : 0
    Failed    : 非0，表示初始化失败

 History    :
    1.Date         : D2013_06_17
     Author       : wutiejun
     Modification : Created function

*****************************************************************************/
int __init hw_ssp_ker_obj_init(void)
{
    int ret = 0;
    HW_SSP_KER_OBJ_MANAGER* pObjManager = NULL;

#ifdef CONFIG_BUILD_PANIC
    ret |= hw_ssp_ker_panic_init();
#endif

    /* 创建全局的容器管理者 */
    g_ObjManager = hw_ssp_ker_obj_create_inner("atp_obj_manager");
    g_ObjManager->ObjCmp = hw_ssp_ker_obj_manager_cmp;
    g_ObjManager->ObjShow = hw_ssp_ker_obj_manager_show;

    /* 创建容器管理对象 */
    pObjManager = hw_ssp_ker_obj_new(sizeof(HW_SSP_KER_OBJ_MANAGER), GFP_KERNEL);
    pObjManager->pObjContainer = g_ObjManager;

    /* 将容器管理对象添加到全局的容器管理者中 */
    ret = hw_ssp_ker_obj_add(g_ObjManager, pObjManager);

#ifdef  CONFIG_BUILD_ATP_MLOG
    ret |=hw_ssp_ker_wq_init();
    ret |=hw_ssp_ker_last_word_init();
#endif
    //proc_mlog_log_init();
    return ret;
}

/*lint -restore*/
subsys_initcall(hw_ssp_ker_obj_init);
/*lint -save -e129 -e19*/
EXPORT_SYMBOL(hw_ssp_ker_obj_new);
EXPORT_SYMBOL(hw_ssp_ker_obj_free);
EXPORT_SYMBOL(hw_ssp_ker_obj_add);
EXPORT_SYMBOL(hw_ssp_ker_obj_del);
EXPORT_SYMBOL(hw_ssp_ker_obj_search);
EXPORT_SYMBOL(hw_ssp_ker_obj_find);
EXPORT_SYMBOL(hw_ssp_ker_obj_create);
EXPORT_SYMBOL(hw_ssp_ker_obj_dispose);
EXPORT_SYMBOL(hw_ssp_ker_obj_all_del_call_back);
EXPORT_SYMBOL(hw_ssp_ker_obj_foreach_del);
EXPORT_SYMBOL(hw_ssp_ker_obj_foreach);
EXPORT_SYMBOL(hw_ssp_ker_obj_first);
EXPORT_SYMBOL(hw_ssp_ker_obj_last);

//EXPORT_SYMBOL(hw_ssp_ker_obj_init);
//EXPORT_SYMBOL(hw_ssp_basic_get_atp_dir);

#endif //Region

