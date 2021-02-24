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

/* �ص����� */
/*******************************************************************************
    ����ȽϺ��������践����������Ĵ�С��ϵ�����pObjA>pObjB�򷵻ش���0������pObjA<pObjB�򷵻�С��0���������򷵻�0
    �������0�����ʾ����������ȫһ�����Ҳ��ܷ���ͬһ��container�С�
*******************************************************************************/
typedef int (*pf_hw_ssp_ker_obj_cmp)(void * pObjA, void * pObjB);
/* 
    ʹ�ö��ַ���container��ͨ��KeyData����ѯ���󣬱��践�ش�С��ϵ
    ���KeyData���ڱȽϵĶ��󣬾ͷ��ش���0��ֵ��KeyDataС�ڶ����򷵻�С��0��ֵ�����򷵻�0
*/
typedef int (*pf_hw_ssp_ker_obj_search_cmp)(void * pObj, int KeyData);

/* seq�ļ�ϵͳ�Ķ�ȡ�ӿڣ�����ʹ��seq�ļ��Ľӿ���m���Ҫ��ʾ������ */
typedef int (*pf_hw_ssp_ker_obj_show)(struct seq_file *m, void * pData,  unsigned int Size);

/* ֧��ioctl�Ľӿ� */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36)
typedef int (*pf_hw_ssp_ker_obj_ioctl)(struct inode *pinode, struct file *pfile,  unsigned int cmd,  unsigned long Arg);
#else
typedef int (*pf_hw_ssp_ker_obj_ioctl)(struct file *pfile,  unsigned int cmd,  unsigned long Arg);
#endif


/* ֧��write�Ľӿ� */
typedef ssize_t (*pf_hw_ssp_ker_obj_write)(struct file * file, const char __user * acBuffer, size_t buflen, loff_t * fpos);

/* ʹ�ñ����ķ�ʽ��ѯ��������ҵ����󣬾ͷ��ط�0ֵ�����ط�0�������� */
typedef int (*pf_hw_ssp_ker_obj_find)(void * pObj, void * pData);
/****************************************************************************************************************/

/***********************************************���ݽṹ****************************************************/
typedef struct hw_ssp_ker_obj_container_
{
    /* ���³�Ա�û����ù�ע��Ҳ��������޸� */
    const char * _ObjProcName;
    struct rb_root _ObjRoot;
    unsigned int _ObjCount;
    unsigned int  _ObjTotalSize;
    struct proc_dir_entry * _ProcEntry;
    struct file_operations _FileOp;
    struct seq_operations _SeqOp;
     unsigned long _SpinFlag;             /* �ж�����־ */
    spinlock_t _ObjLocker;          /* ��ʹ�ù����жϵ����������ڶ�ʱ��ʹ�øû��� */
    void * _pLastObjShow;

    /* ÿ�м�¼��ӡ�Ĵ�С����Seq�ļ�����С�ڸ�ֵʱ���ͷ�ҳ������û�����ÿ����¼�������С�����ø�ֵ��Ĭ��128�����ù�С���޷���ӡ����ˡ� */
    unsigned int MaxShowSize;
        
    /* �Ƚϻص�����������ڣ����践�ش�С��ϵ�������Ⱦͱ�ʾ��ͬһ�����󣬼��ڴ��ַ����һ���� */
    pf_hw_ssp_ker_obj_cmp ObjCmp;       /* ����ȽϺ��������ڱȽ���������Ĵ�С����Ҫ���ش�С��ϵ */

    /* ���º�������Ϊ�գ����Ϊ�գ����Ӧ�Ĺ��ܲ�֧�� */
    pf_hw_ssp_ker_obj_show ObjShow;     /* ͨ��proc�ļ�����ȡ������Ϣʱ��ʾ����pDataΪ��ʱ����ʾ��ͷ�����Ϊ�վͲ�֧��proc��ȡ */
    pf_hw_ssp_ker_obj_ioctl ObjIoctl;   /* ioctl�����������Ϊ����֧��ioctl */
    pf_hw_ssp_ker_obj_write ObjWrite;   /* д����֧�֣����Ϊ�գ���֧��д���� */
    //
    pf_hw_ssp_ker_obj_search_cmp ObjSearcher;   /* �����ѯ�ص����������ʹ��Ĭ�ϵ�ioctl������ʵ�ָú��� */
    //
} hw_ssp_ker_obj_container;	
/****************************************************************************************************************/

#ifdef __cplusplus
}
#endif

#endif 
