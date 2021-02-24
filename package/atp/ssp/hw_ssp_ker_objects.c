

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

/* obj�ڵ����ݣ�pObjָ��ʵ�ʵ����ݣ����ݸ�ʽΪ����ʽ(��ָ��������ʵ������) */
typedef struct tagHW_SSP_KER_OBJ_NODE
{
    unsigned short ObjMagicFlag;       /* ���ڱ�ʶ����ڴ��Ƿ���ͨ��Obj��������� */
    unsigned short ObjSize;
    struct rb_node node;
    void* pObj;
} HW_SSP_KER_OBJ_NODE;

/* obj���Թ���������ڹ���HW_SSP_KER_OBJ_CONTAINER */
typedef struct tagHW_SSP_KER_OBJ_MANAGER
{
    hw_ssp_ker_obj_container* pObjContainer;
} HW_SSP_KER_OBJ_MANAGER;

 void* hw_ssp_ker_obj_search(hw_ssp_ker_obj_container* pstContainer,
                                   pf_hw_ssp_ker_obj_search_cmp Searcher, int KeyData);


/* Ψһ��ȫ�ֱ��������ڹ������е�obj���� */
hw_ssp_ker_obj_container* g_ObjManager = NULL;

/* �ڲ���ѯ������������ϸ�μ�����ע�� */
PRIVATE int hw_ssp_ker_obj_manager_search(void* pObj, int KeyData);
extern int __init hw_ssp_ker_wq_init(void);
extern int __init hw_ssp_ker_last_word_init(void);
extern int hw_ssp_ker_panic_init(void);
void hw_ssp_ker_obj_dispose(hw_ssp_ker_obj_container* pstContainer);

/*��ȡ�ַ�����Ŀ���ַ���λ��*/
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

#if  Region("���������")
/* ����Ӧ�ó���������������в��ܼ�¼LSW */

static void hw_ssp_ker_obj_lock(hw_ssp_ker_obj_container* pObjInfo)
{
    spin_lock_irqsave(&pObjInfo->_ObjLocker, pObjInfo->_SpinFlag);
    return;
}

/*****************************************************************************
 Function name: hw_ssp_ker_obj_unlock
 Description  : ����������װ��
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
 Description  : ��������ӵ������С�����ʹ�ú��������
 Input        : hw_ssp_ker_obj_container * pObjInfo  ��������
                HW_SSP_KER_OBJ_NODE * pAddNode       ��Ҫ��ӵĶ���
 Output       : None
 Return Value :
    Successed : 0
    Failed    : ������

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

    /* ������������ҵ���Ӧ�Ľڵ� */
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
            /* ͬһ�����У����ܴ���Key��ͬ���������󣬼�¼LSW�����ش��� */
            hw_ssp_ker_obj_unlock(pObjInfo);
            HW_SSP_KOBJ_LOG(HW_RET_OS_ADDR_EXIST, pAddNode->pObj, pstTmNode->pObj,
                            pAddNode, pObjInfo);
            return (int)HW_RET_OS_ADDR_EXIST;
        }
    }

    /* ���ڵ���ӵ�����������¶Ժ������ɫ��ʹ֮ƽ�� */
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
 Description  : ��������ȡ��ȡ�õ����ã�ʹ�ö���ʱ�Ѿ��������ˣ�
                ��Ҫע�����ҵ����Ҫ����Ҫ���������
 Input        : hw_ssp_ker_obj_container * pObjContainer  ����
                pf_hw_ssp_ker_obj_search_cmp Searcher     ��ѯ�Ļص�����
                unsigned int KeyData                           ��ѯ��Key����
 Output       : None
 Return Value :
    Successed : ��������õ�ַ
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

#ifdef OBJ_SUOPORT_IOCTL /*��δʵ��*/
typedef int (*pf_hw_ssp_ker_obj_ioctl_call)(hw_ssp_ker_obj_container* pContainer, HW_SSP_KER_OBJ_IOCTL* pstCtrl);
#if Region("֧��IOCTL����:")
/* ioctrl�����п���ʹ��lastword��¼���� */

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

    /* ʹ�ü򵥵ı�������ʵ��ioctl����ַ� */
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

#if Region("�ļ�����:")
/*****************************************************************************
 Function name: hw_ssp_ker_obj_get_container
 Description  : ͨ���򿪵��ļ���Ϣȡ��ϵͳ��������
 Input        : struct file *pfile  ָ��procĿ¼�µ��ļ�����
 Output       : None
 Return Value :
    Successed : �����������õ�ַ
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
 Description  : ���ݲ��ҵ��������ص���ͬ��ioctl��
 Input        : struct inode *pinode  procĿ¼�µ��ļ��ڵ�
                struct file *pfile    procĿ¼�µ��ļ�����
                unsigned int cmd           ioctl������������
                unsigned long arg          ioctl�����Ĳ���
 Output       : None
 Return Value :
    Successed : 0
    Failed    : ������

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

    /* �ڲ��Ѿ�֧�ֵ�ioctl������֧�����ݵ���ӡ�ɾ�����޸ĺͲ�ѯ */


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
 Description  : ���ݴ򿪵Ĳ�ͬ�ļ����ҵ���Ӧ��������Ȼ���seq�ļ���
 Input        : struct inode *pinode    procĿ¼�µ��ļ��ڵ�
                struct file *filp       procĿ¼�µ��ļ�����
 Output       : None
 Return Value :
    Successed : 0
    Failed    : ������

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

    /* ��seq�ļ���˽����������Ϊ������Ҫʹ�õĶ������� */
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
 Description  : procĿ¼�µ��ļ�д������
 Input        : struct file * file              procĿ¼�µ��ļ�����
                const char __user * acBuffer    �û�д������ݻ�������ָ������û��ռ�
                size_t buflen                   д��Ļ�������С
                loff_t * fpos                   д���ļ���ƫ��
 Output       : None
 Return Value :
    Successed : д���ļ������ݴ�С
    Failed    : д���ļ������ݴ�С��һ��Ϊ0��ʾд��ʧ��

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

    /* ������󲻴��ڣ�����û�йһ�д��������ֱ�ӷ���0����ʾ��֧��д���� */
    pContainer = hw_ssp_ker_obj_get_container(file);
    if (unlikely(NULL == pContainer))
    {
        return 0;
    }

    if (NULL == pContainer->ObjWrite)
    {
        return 0;
    }

    /* ��ȷ�ص�д������������Ҫ����д����ֽ��� */
    ret = pContainer->ObjWrite(file, acBuffer, buflen, fpos);

    return ret;

}

/*****************************************************************************
 Function name: hw_ssp_ker_obj_seq_start
 Description  : seq�ļ�ϵͳ�Ŀ�ʼ��ȡ��
 Input        : struct seq_file *m      seq�ļ�����
                loff_t *pos             �ļ�ƫ��
 Output       : None
 Return Value :
    Successed : �����ڶ�������õ�ַ��������ʼ��־SEQ_START_TOKEN
    Failed    : NULL����ʾ������û�ж���

 History    :
    1.Date         : D2013_06_18
     Author       : wutiejun
     Modification : Created function

*****************************************************************************/
PRIVATE void* hw_ssp_ker_obj_seq_start(struct seq_file* m, loff_t* pos)
{
    hw_ssp_ker_obj_container* pContainer = NULL;
    pContainer = (hw_ssp_ker_obj_container*)m->private;

    /* �ں˻ᱣ֤����hw_ssp_ker_obj_seq_start��һ������hw_ssp_ker_obj_seq_stop�����Խ��� */
    hw_ssp_ker_obj_lock(pContainer);

    /* ���û�й�show��������ֱ�ӷ��� */
    if (NULL == pContainer->ObjShow)
    {
        return NULL;
    }

    /* ��һ�ο�ʼ��ȡʱposΪ0 */
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
 Description  : seq�ļ�ϵͳ��ֹͣ��
 Input        : struct seq_file *m  seq�ļ�����
                void *v          �ϴζ�ȡ�������еĶ���һ�㲻ʹ��
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
 Description  : seq�ļ�����һ����ȡ������
 Input        : struct seq_file *m      seq�ļ�����
                void *v              �����еĵ�ǰ����
                loff_t *pos             �ļ�ƫ��
 Output       : None
 Return Value :
    Successed : �����е���һ������
    Failed    : NULL����ʾ�������Ѿ�û�ж���

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
 Description  : Seq�ļ���show������ͨ��private�ֶ�ȡ�����������ص���ص�show
                ������
 Input        : struct seq_file *m      seq�ļ�����
                void *v              ��ǰ��ȡ�������еĶ���
 Output       : None
 Return Value :
    Successed : 0
    Failed    : ��0����ʾ���ʧ�ܣ��´ζ�ȡ�������ӵ�ǰ�������

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
        /* �����ȡ�Ķ���Ϊ�գ�����û�й�show������ֱ�ӷ��أ������ */
        return 0;
    }

    if (SEQ_START_TOKEN == v)
    {
        /* ����ǳ�ʼ��־��ʹ��NULL���������������ʾ��Ҫ�����ͷ���� */
        pContainer->ObjShow(m, NULL, 0);
        return 0;
    }

    pObjNode = (HW_SSP_KER_OBJ_NODE*)v;

    /* �����ӡ���������󻺴�ʹ�ӡ�����������ʱ�����������һ����� */
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
procĿ¼�¶�����ļ������ṹ�����е��ļ������������������
���������ٵ���Ҫʵ�ֵĻص��������ļ�������������������ʵ�֡�
*******************************************************************************/
struct file_operations hw_ssp_ker_obj_proc_op =
{
    .owner = THIS_MODULE,
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36)	
    .ioctl  = hw_ssp_ker_obj_ioctl,     /* ioctl�����ص����� */
#else
    .unlocked_ioctl  = hw_ssp_ker_obj_ioctl,     /* ioctl�����ص����� */
#endif
    .open = hw_ssp_ker_obj_open,        /* open�����ص����� */
    .write = hw_ssp_ker_obj_write,      /* write�����ص����� */
    .read = seq_read,                   /* read�����ص����� */
    .release = seq_release,             /* release�����ص����� */
    .llseek = seq_lseek,                /* llseek�����ص����� */
};

#endif //Region

#if  Region("����API")
/* ����Ӧ�ó������ں�API�в��ܼ�¼LSW */
/*****************************************************************************
 Function name: hw_ssp_ker_obj_new
 Description  : ����һ��������ӵ������еĶ���
 Input        : int ObjSize      ����Ĵ�С
                int Flag         �ں������ڴ��Flag�����ڱ�־�����ڴ�ʱ�Ƿ��������
 Output       : None
 Return Value :
    Successed : ������ڴ��ַ
    Failed    : NULL�������ڴ�ʧ��

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
        /* ����޷����뵽�ڴ棬����SEҪ��ֱ��panic��λ�����������Ʒ�����λ�ڴ����� */
        panic("hw_ssp_ker_obj_new:out of memory!:size=%d;flag=%d;\r\n", TotalSize, Flag);
        return NULL;
    }

    memset(pObjNode, 0, TotalSize);
    pObjNode->ObjSize = (unsigned short)TotalSize;  /* �������¼�·��������λ��ʹ��ushortû������ */
    pObjNode->pObj = (void*)((unsigned int)pObjNode + sizeof(HW_SSP_KER_OBJ_NODE));

    return pObjNode->pObj;

}

/*****************************************************************************
 Function name: hw_ssp_ker_obj_free
 Description  : �ͷ�һ�����󣬵���ӵ�������ʧ�ܣ����߲���ʹ�øö���ʱ����Ҫ
                �ͷš����ҵ����Ҫ����Ҫ����ҵ������
 Input        : void * pObj      ʹ��hw_ssp_ker_obj_new���ɵĶ���
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
        /* �ö�����ĳ�������У�����free������Ҫ�ȴ�������ɾ���Ժ����ͷ� */
        HW_SSP_KOBJ_LOG(HW_OS_RT_ARGC_ERROR, pObj, pObjNode->ObjMagicFlag, 0, 0);
        return;
    }

    /* freeǰ����Ϊ0�����ҵ���ͷź���ʹ���ڴ棬�ͻ���Ϊ��ָ���쳣��������ǰ��¶���� */
    memset(pObjNode, 0, sizeof(HW_SSP_KER_OBJ_NODE));

    kfree(pObjNode);

    return;

}

/*****************************************************************************
 Function name: hw_ssp_ker_obj_add
 Description  : ��������ӵ�ָ�������С�
 Input        : hw_ssp_ker_obj_container * pObjContainer    ��������
                void * pObj                              ��Ҫ��ӵ������еĶ���
 Output       : None
 Return Value :
    Successed : 0
    Failed    : ������

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
        /* �Ѿ���ĳ���������ˣ������ظ���� */
        HW_SSP_KOBJ_LOG(HW_OS_RT_ARGC_ERROR, pObjContainer, pObjNode->ObjMagicFlag, 0, 0);
        return (int)HW_OS_RT_ARGC_ERROR;
    }

    return hw_ssp_ker_obj_add_to_rbtree(pObjContainer,  pObjNode);
}

/*****************************************************************************
 Function name: hw_ssp_ker_obj_del
 Description  : ��ָ��������ɾ������
 Input        : hw_ssp_ker_obj_container * pObjContainer  ��������
                void * pObj                            ��Ҫɾ���Ķ���
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
 Description  : ��ָ��������ɾ������
 Input        : hw_ssp_ker_obj_container * pObjContainer  ��������
                void * pObj                            ��Ҫɾ���Ķ���
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
        /* �������ڱ����� */
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
 Description  : ��ָ��������ͨ��KEYֵ���Ҷ�����Ҫ�ص������search��������
                ��û��ָ��search�������򷵻�NULL��ʹ�õ��Ƕ��ֲ��ҡ�
                ���Searcher���践����ȷ�Ĵ�С��ϵ�������޷���ѯ��
 Input        : hw_ssp_ker_obj_container * pstContainer     ��������
                pf_hw_ssp_ker_obj_search_cmp Searcher       ��ѯ����
                unsigned int KeyData                             �����keyֵ
 Output       : None
 Return Value :
    Successed : ��ѯ���Ķ������õ�ַ
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
 Description  : ʹ�ñ����ķ�ʽ��ѯ����������ж���
                Finderֻ���ж��Ƿ�Ϊ�ҵ��Ķ��󼴿ɣ�������ҵ��Ķ��󣬷��ط�0�����򷵻�0
 Input        : hw_ssp_ker_obj_container * pstContainer     ��������
                pf_hw_ssp_ker_obj_search where              ��ѯ�ص�����
                void * pData                             ��ѯ����
 Output       : None
 Return Value :
    Successed : ��������õ�ַ
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
 Description  : ���ڶ���ȫ��ɾ���Ļص�API��
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
 Description  : ɾ����������Finder�����Ķ���ÿɾ��һ��ʱ������¿�ʼ������
                Ч�ʱȽϵ͡�������ʹ�ã���Ӧ��ʹ��ָ�������ɾ��������
 Input        : hw_ssp_ker_obj_container * pstContainer     ��������
                pf_hw_ssp_ker_obj_find Finder               ɾ��ʱ�ıȽϲ�ѯ������
                                                            ���ú������ط�0ʱ����ʾɾ����ѯ���Ķ���
                void * pData                             ��ѯ����
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
            /* ɾ����ǰ�ڵ� */
            hw_ssp_ker_obj_del_inner(pstContainer, pObjNode);
            hw_ssp_ker_obj_free(pObjNode->pObj);

            /* ɾ���Ժ󣬺�����仯����Ҫ���´Ӹ��ڵ���� */
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
 Description  : ѭ�������������ÿһ�����󣬲����ڻص��������ٲ�������������
                ������
 Input        : hw_ssp_ker_obj_container * pstContainer  ��������
                pf_hw_ssp_ker_obj_find ForeachCallBack   �����ص�����
                void * pData                          �ص�����
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
 Description  : ���������ĳ�ʼ����
 Input        : const char * _ObjProcName    ����������
 Output       : None
 Return Value :
    Successed : ��������������
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
        /* ��������ʧ�ܣ�һ���Ƕ������ظ� */
        HW_SSP_KOBJ_LOG(HW_OS_RT_ARGC_ERROR, 0, 0, 0, 0);
        return NULL;
    }

    /* �����ڴ沢��ʼ���ṹ */
    pstObjInfo = kmalloc(sizeof(hw_ssp_ker_obj_container), GFP_KERNEL);
    if (unlikely(NULL == pstObjInfo))
    {
        /* ��������ʧ�ܣ�ϵͳ�޷����У�ֱ��panic */
        panic("hw_ssp_ker_obj_create_inner:out of memory!:size=%d;flag=%d;\r\n",
              sizeof(hw_ssp_ker_obj_container), GFP_KERNEL);
        return NULL;
    }

    memset(pstObjInfo, 0, sizeof(hw_ssp_ker_obj_container));

    spin_lock_init(&pstObjInfo->_ObjLocker);
    pstObjInfo->_ObjProcName = ObjProcName;
    pstObjInfo->_ProcEntry = pTempProc;
    //
    //del this proc_create ���һ���������������ĸ�ֵ
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
 Description  : ����API�����ڴ�����������ͬʱ���������󵱳�obj-manager����
                ����ӵ�ȫ�ֵĹ���������
 Input        : const char * ObjProcName  ������
 Output       : None
 Return Value :
    Successed : ��������������
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

    /* ���ⴴ���Ķ�����ӵ�ȫ�ֵ�obj������ */
    pObjManager = (HW_SSP_KER_OBJ_MANAGER*)hw_ssp_ker_obj_new(sizeof(HW_SSP_KER_OBJ_MANAGER), GFP_KERNEL);
    pObjManager->pObjContainer = pstObjContainer;

    /* ����������Ϊ������ӵ�ȫ�ֵ������У��γɺϳɵĹ���ģʽ */
    ret = hw_ssp_ker_obj_add(g_ObjManager, pObjManager);
    if (unlikely(ret != 0))
    {
        hw_ssp_ker_obj_free(pObjManager);
        hw_ssp_ker_obj_dispose(pstObjContainer);
        HW_SSP_KOBJ_LOG(ret, 0, 0, 0, 0);
        return NULL;
    }

    /* ���ش��������� */
    return pstObjContainer;

}

/*****************************************************************************
 Function name: ����������ַ��ѯ���������ߡ�
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
 Description  : �ͷ�����������
 Input        : hw_ssp_ker_obj_container * pstContainer
                ��ʱpstContainer��ʵ����g_ObjManager��һ�����󣬿��Ե��ɶ���ʹ�á�
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

    /* ͨ������ַַ�ҵ����������� */
    pManager = hw_ssp_ker_obj_find(g_ObjManager, hw_ssp_ker_obj_dispose_finder, pstContainer);
    if (unlikely(NULL == pManager))
    {
        HW_SSP_KOBJ_LOG(HW_OS_RT_ARGC_ERROR, pstContainer, 0, 0, 0);
        return;
    }

    /* �Ƚ������ӹ���������ɾ�� */
    hw_ssp_ker_obj_del(g_ObjManager, pManager);

    hw_ssp_ker_obj_lock(g_ObjManager);

    /* �ٱ���ɾ�������������ж��� */
    next_node = rb_first(&pstContainer->_ObjRoot);
    while (next_node)
    {
        pObjNode = rb_entry(next_node, HW_SSP_KER_OBJ_NODE, node);
        hw_ssp_ker_obj_del_inner(pstContainer, pObjNode);
        hw_ssp_ker_obj_free(pObjNode->pObj);

        /* ÿ�ζ�����һ���ڵ�ɾ���������ڵ����ò����� */
        next_node = rb_first(&pstContainer->_ObjRoot);
    }

    /* ɾ��proc�ļ� */
    pAtpProc = hw_ssp_basic_get_atp_dir();
    remove_proc_entry(pstContainer->_ObjProcName, pAtpProc);

    hw_ssp_ker_obj_unlock(g_ObjManager);

    /* �ͷ��ڴ棬��������ָ��Ϊ�� */
    hw_ssp_ker_obj_free(pManager);

    return;

}

/*****************************************************************************
 Function name: hw_ssp_ker_obj_first
 Description  : ȡ�����ڵ�һ����Ԫ�ء����Ժ����׵�ʵ�ֶ��С�
 Input        : hw_ssp_ker_obj_container * pstContainer
 Output       : None
 Return Value :
    Successed : ��������õ�ַ
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
 Description  : ȡ�����һ��Ԫ�ء����Ժ����׵�ʵ��ջ��
 Input        : hw_ssp_ker_obj_container * pstContainer
 Output       : None
 Return Value :
    Successed : ��������õ�ַ
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

#if  Region("�����������")
/*****************************************************************************
 Function name: hw_ssp_ker_obj_manager_cmp
 Description  : obj���������ıȽϺ������Ƚ���������Ĵ�С��
 Input        : void * pObjA     ����A
                void * pObjB     ����B
 Output       : None
 Return Value : A��B��������Ĵ�С��ϵ�����A����B�򷵻����������ڷ���0�����򷵻ظ���

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
 Description  : ���������Ĳ�ѯ���������ڶ��ֲ�ѯ�Ļص���
 Input        : void * pObj      ���ڲ�ѯ�Ķ���
                int KeyData      ��ѯKey����
 Output       : None
 Return Value : ��ѯ������Key�Ĵ�С��ϵ������������key���򷵻�������
                ��ȷ���0�����򷵻ظ�����

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
 Description  : ����������show
 Input        : struct seq_file *m      seq�ļ�����
                void * pData         ��Ҫshow�Ķ���
                unsigned int Size            ����Ĵ�С
 Output       : None
 Return Value :
    Successed : 0
    Failed    : ��0����ʾ�ö���showʧ�ܣ��´μ���show�ö���

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
        /* ���Title */
        seq_printf(m, "Obj-Num  Total-Size Address    Obj-Name \r\n");
        return 0;
    }
    else
    {
        /* ������� */
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
 Description  : ��ʼ��objects���������������Լ���ӵ�����������
 Input        : void
 Output       : None
 Return Value :
    Successed : 0
    Failed    : ��0����ʾ��ʼ��ʧ��

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

    /* ����ȫ�ֵ����������� */
    g_ObjManager = hw_ssp_ker_obj_create_inner("atp_obj_manager");
    g_ObjManager->ObjCmp = hw_ssp_ker_obj_manager_cmp;
    g_ObjManager->ObjShow = hw_ssp_ker_obj_manager_show;

    /* ��������������� */
    pObjManager = hw_ssp_ker_obj_new(sizeof(HW_SSP_KER_OBJ_MANAGER), GFP_KERNEL);
    pObjManager->pObjContainer = g_ObjManager;

    /* ���������������ӵ�ȫ�ֵ������������� */
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

