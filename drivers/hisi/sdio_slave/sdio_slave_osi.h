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

#ifndef _SDIO_SLAVE_OSI_H
#define _SDIO_SLAVE_OSI_H

#ifdef __cplusplus
extern "C" {
#endif

/*lint -e413 -e950 -e528*/
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/string.h>
#include <mach/hardware.h>
#include <asm/cacheflush.h>
#include <asm/io.h>
#include <linux/dma-mapping.h>
#include "drv_comm.h"
#define	bzero(b, len)		memset((b), '\0', (len))

#ifndef LOCAL
#define LOCAL static
#endif

#ifndef BSP_MIN
 #define BSP_MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif
#define SLAVE_FLUSH_CACHE(ptr,size)  __dma_single_cpu_to_dev(ptr, size, 1)

#define SLAVE_INVALID_CACHE(ptr,size)  __dma_single_dev_to_cpu(ptr,size,2)

#define SLAVE_VIRT_PHY(virt) virt_to_phys((void*)virt)

#define SLAVE_PHY_VIRT(phy) phys_to_virt((unsigned long)phy)

#define LIST_NODE_NUM  10

/* Cache align ����mask */
//#define SLAVE_CACHE_ALIGN       (_CACHE_ALIGN_SIZE)
//#define SLAVE_CACHE_ALIGN_MASK  (_CACHE_ALIGN_SIZE-1)
#define SLAVE_UNALIGN_ASSIST_BUFF_LEN (16 * 1024)
#define SLAVE_CACHE_ALIGN      (32)
#define SLAVE_CACHE_ALIGN_MASK   (32-1)

#define NOT_CACHE_ALIGN(value) (((BSP_U32)(value)) & (SLAVE_CACHE_ALIGN_MASK))
#define CACHE_ALIGN_SIZE(size) (((BSP_U32)(size)+SLAVE_CACHE_ALIGN) & (SLAVE_CACHE_ALIGN_MASK))

//host���͸�slave����Ϣ����,����ͨ����Ϣ��ʽ���Ϳ�����Ϣ���ȡSLAVE��״̬��Ϣ
//#define MSG_READ_REQUEST        0x90008001          /*����������*/
#define MSG_BUFF_SIZE_GET       0x90008001
#define MSG_MODE_SWITCH_SD20    0x90008002          /*�����л�*/
#define MSG_MODE_SWITCH_SD30    0x90008003          
#define MSG_MODE_SWITCH_UHS     0x90008004         /*�Ƿ��л���UHSģʽ*/
#define MSG_STATUS_GET          0x90008005         /*��ȡslave״̬��Ϣ*/
#define MSG_ERROR_CODE_GET      0x90008006         /*��ȡ��������*/
#define MSG_CSA_SUPPORT         0x90008007         /*����֧��CSA*/
#define  RECV_BUFF_RESET            0x90008008
/* IOCTL CMD ���� */
#define SLAVE_IOCTL_SET_WRITE_CB      0x80001000
#define SLAVE_IOCTL_SET_READ_CB       0x80001001
#define SLAVE_IOCTL_GET_STATUS        0x80001002
#define SLAVE_IOCTL_GET_BUFFINFO      0x80001003
#define SLAVE_IOCTL_RET_BUFF          0x80001004
#define SLAVE_CLR_STAT_CNT            0x80001005 /*���״̬����*/

/*********Definition of Data Types used*************/
typedef union tagAHBFun0Int_REG_U
{
    BSP_U32 u32Reg;
    struct Fn0bitsTag
    {
        BSP_U32 soft_rst                :  1;  //LSB
        BSP_U32 volt_sw_cmd             :  1;
        BSP_U32 cmd19_rd_strt           :  1;
        BSP_U32 cmd19_rd_trans_over     :  1;
        BSP_U32 fn0_wr_start            :  1;
        BSP_U32 fn0_wr_trn_over         :  1;
        BSP_U32 fn0_rd_start            :  1;
        BSP_U32 fn0_rd_trn_over         :  1;
        BSP_U32 fn0_rd_trn_err          :  1;
        BSP_U32 fn0_adma_end_int        :  1;
        BSP_U32 fn0_adma_int            :  1;
        BSP_U32 fn0_adma_err            :  1;
	 BSP_U32 Reserved			 :20;
    } Fn0Bits;   /* struct */
} AHBFun0Int_REG;

typedef union tagAHBFun1Int_REG_U
{
    BSP_U32 u32Reg;
    struct Fn1bitsTag
    {
        BSP_U32 fn_wr_over             :  1;  //LSB
        BSP_U32 fn1_rd_over            :  1;
        BSP_U32 fn1_rd_error           :  1;
        BSP_U32 fun1_rst               :  1;
        BSP_U32 sd_host_fn1_msg_rdy    :  1;
        BSP_U32 fn1_ack_to_arm         :  1;
        BSP_U32 fn1_sdio_rd_start      :  1;
        BSP_U32 fn1_sdio_wr_start      :  1;
        BSP_U32 adma_end_int           :  1;
        BSP_U32 fn1_suspend            :  1;
        BSP_U32 resume                 :  1;
        BSP_U32 adma_int               :  1;
        BSP_U32 adma_err               :  1;
        BSP_U32 fun1_en                :  1;
        BSP_U32 Reserved2              :  18;
    } Fn1Bits;   /* struct */
} AHBFun1Int_REG;

/*�ж�״̬*/						   
typedef struct tagSLAVE_INTMSG_S
{
    BSP_U32 global;
    BSP_U32 func0;
    BSP_U32 func1;
    BSP_U32 mem;
}SLAVE_INTMSG_S;

/* ��buffer��Ϣ */
typedef struct tagSLAVE_RX_BUFF_INFO_S
{
    BSP_U8* pBuffer;
    BSP_U32 u32Size;
    void* pDrvPriv;
}SLAVE_RX_BUFF_INFO_S;

typedef enum tagSLAVE_STAT_E
{   
    TRANS_IDLE = 0,            /*IDLE״̬*/
    DATA_SEND,                 /*slave -> host*/
    DATA_READ,                /*host -> slave */
    RECV_BUFF_EMPTY,         /*���ݶ���*/
    RECV_BUFF_FULL,           /*����д��*/
    STAT_MAX
}SLAVE_STAT_E;


struct arasan_sido_log_desc {
	u32		addr;
	u32		len;
	u8		dir;
};

struct arasan_sido_log {
	struct arasan_sido_log_desc desc[10];
	u8 		curr;
};
#if 0
typedef BSP_S32 (*FnEnumCfg)(BSP_VOID); /*�豸ö��*/
typedef BSP_VOID (*SLAVE_READ_DONE_CB_T)(); /*����ɻص�*/
typedef BSP_VOID (*SLAVE_WRITE_DONE_CB_T)();/*д��ʼ�ص�*/

/* send list ѭ��˫������*/
typedef struct tagSLAVE_ITEM_S 
{
    BSP_VOID        *addr;
    BSP_VOID        *addrPhy;
    BSP_U32         dlen;
} SLAVE_ITEM_S;

/* ����ڵ���Ϣ */
typedef struct tagSLAVE_NODE_INFO
{
    struct list_head stList;
    SLAVE_ITEM_S stItem;
}SLAVE_NODE_INFO;

#define SLAVE_BLOCK_T struct semaphore

typedef struct tagSLAVE_RX_PRIV_S
{
    SLAVE_READ_DONE_CB_T readDoneCB;
    SLAVE_BLOCK_T       stRdBlockSem;//�������ź���
}SLAVE_RX_PRIV_S;


typedef struct tagSLAVE_TX_PRIV_S
{
    SLAVE_NODE_INFO *pNodeInfo;
    SLAVE_NODE_INFO *pCurNode;
    struct list_head free_list;
    struct list_head use_list;
    BSP_U32 u32NodeNo;
    BSP_U32 u32NodeInUse;                  /*ʹ���е�����ڵ���*/
    SLAVE_WRITE_DONE_CB_T writeDoneCB;
    SLAVE_BLOCK_T stWrBlockSem;            /*д�����ź���*/
    BSP_BOOL bCached;
}SLAVE_TX_PRIV_S;


/*���ݽ���Buffer*/
typedef struct tagSLAVE_RX_SUB_S
{
    BSP_U32    TotalLen;            /*��Ч���ݴ�С*/
    BSP_U32    WrInLen;             /*hostд������ݳ���*/
    BSP_U8     *pWrite;             /*дָ��*/
    BSP_U8     *pRead;              /*��ָ��*/
    BSP_U8     *pStart;             /*buffer����ʼ��ַ*/
}SLAVE_RX_SUB_S;


/* SLAVE�豸�ṹ�� */
typedef struct tagSLAVE_DEV_S
{
    BSP_U32 intLvl;
    BSP_U32 RegBase;
    BSP_U32 SlaveCatalog;
    FnEnumCfg pFnEnumCfg;
    SLAVE_RX_PRIV_S stReadPrv;
    SLAVE_TX_PRIV_S stWritePrv;
    SLAVE_RX_SUB_S *stReadCntl;       /*����buffer����*/

    BSP_U8 *pCis0DataBuffer;         /*Cis0buffer*/
    BSP_U8 *pCis0DataBuffer_NotAlign;
    dma_addr_t  pCis0DataBufferPhy;
    dma_addr_t  pCis0DataBufferPhy_NotAlign;

    BSP_U8 *pCis1DataBuffer;         /*Cis1������*/
    BSP_U8 *pCis1DataBuffer_NotAlign;
    dma_addr_t  pCis1DataBufferPhy;
    dma_addr_t  pCis1DataBufferPhy_NotAlign;

    BSP_U8 *pCsaDataBuffer;         /*1K csa buffer*/
    BSP_U8 *pCsaDataBuffer_NotAlign;
    dma_addr_t  pCsaDataBufferPhy;
    dma_addr_t  pCsaDataBufferPhy_NotAlign;

    BSP_U8 *pRecvDataBuff;          /*32K data recv buffer*/
    BSP_U8 *pRecvDataBuff_NotAlign;
    dma_addr_t  pRecvDataBuffPhy; 
    dma_addr_t  pRecvDataBuffPhy_NotAlign;
    
    BSP_U32    pCSALast;              /*��һ�ΰ������CSAָ��*/
    ADMA_HANDLE_S *stAdmaInfoHandle;  /*FUN0������*/
    ADMA_HANDLE_S *stAdmaRxHandle;    /*���ݽ���������*/
    ADMA_HANDLE_S *stAdmaSendHandle;  /*���ݽӷ���������*/
    SLAVE_STAT_S *pstSlaveStat;       /*״̬��Ϣ*/
    BSP_U32     msgCurrent;           /*��ǰ��Ϣֵ*/
}SLAVE_DEV_S;

/*SLAVE �����Ľṹ��*/
typedef struct tagSLAVE_CTX_S
{
    BSP_BOOL bSlaveInit;
//    OS_MSG_TYPE msg_id;
    BSP_S32 task_id;
    SLAVE_DEV_S *pstSlaveDev;

    struct work_struct Fstwork;
    SLAVE_INTMSG_S IntMsg;
}SLAVE_CTX_S;
#endif

/*SLAVE״̬�ṹ��*/
typedef struct tagSLAVE_STAT_S
{
    BSP_U32         isr_total;
    BSP_U32         isr_start1_int;
    BSP_U32         isr_end1_int;
    BSP_U32         isr_over1_int;
    BSP_U32         isr_reset_int;     
    BSP_U32         msg_receive_fail;
    BSP_U32         readCB_cnt;
    BSP_U32         writeCB_cnt;
    BSP_U32         dma_err_int;
    BSP_U32         err_code;      /*������:�������󣬵�ַ����,�������ָ�룬��ʱ�����*/    
    SLAVE_STAT_E    statofCur;     /*��ǰ״̬*/
}SLAVE_STAT_S;

/*SLAVE�ڴ�ʹ������*/
typedef enum tagMEM_SDIO_TYPE
{
    MEM_SDIO_DDR_CACHE_POOL = 0,
    MEM_SDIO_DDR_NOCACHE_POOL,
    MEM_SDIO_AXI_POOL,
    MEM_SDIO_ACP_MODEM,
    MEM_SDIO_ACP_APP,
    MEM_SDIO_POOL_MAX
}MEM_SDIO_POOL_TYPE;

#define HSKEEP_CNT_MAX              0x1

/* ��ȡ�ֽڶ��� */
#define ALIGN_BYTES_APPEND(len) \
    (((len)+SLAVE_CACHE_ALIGN_MASK)/SLAVE_CACHE_ALIGN*SLAVE_CACHE_ALIGN)
    
/* ��ȡ�ڵ���� */
#define ADMA_GET_NODE_COUNT(len) \
    (((len)+(ADMA_BUF_MAXSIZE-1))/ADMA_BUF_MAXSIZE)

/* ����ַ�Ƿ���� */
#define ADMA_CHECK_ALIGN(buf) \
    (0==(buf)%SLAVE_CACHE_ALIGN)

/* ��ȡ����ĵ�ַ */
#define ADMA_GET_ALIGN(buf) \
    ((buf)-(buf)%SLAVE_CACHE_ALIGN)

/* ���������벿�ֺ�ĵ�ַ */
#define ADMA_SKIP_NOALIGN(buf) \
    (ADMA_CHECK_ALIGN(buf)?(buf):(ADMA_GET_ALIGN(buf)+SLAVE_CACHE_ALIGN))

/* ��ȡ�������ֽڸ��� */
#define ADMA_GET_NOALIGN_BYTES(buf) \
    (ADMA_SKIP_NOALIGN(buf)-(buf))

/* ��ȡ�����ֽڸ��� */
#define ADMA_GET_ALIGN_BYTES(buf,len) \
    (len-ADMA_GET_NOALIGN_BYTES(buf))
    
#define ADMA_GET_ASSIST_DATA(pstIdmacHandle,u32buf,u32len) \
    memcpy((BSP_VOID *)u32buf,(BSP_VOID *)pstIdmacHandle->assist_buf,(u32len))

#ifdef ADMA_DESC_MEM_CACHEABLE    
#define ADMA_GET_DESC_COUNT(buf,len) \
    (ADMA_CHECK_ALIGN(buf)\
        ?ADMA_GET_NODE_COUNT(len)\
        :(ADMA_GET_NODE_COUNT(ADMA_GET_ALIGN_BYTES((buf),(len)))+1))

#else
#define ADMA_GET_DESC_COUNT(buf,len)  ADMA_GET_NODE_COUNT(len)
#endif
#define ADMA_FLUSH_DESC(pstAdmahandle,StartIdxTag,StartDescTag,num,flag) \
    if((flag))\
    {\
        if(StartIdxTag+num<=pstAdmahandle->DescCount)\
        {\
           (void)SLAVE_FLUSH_CACHE((BSP_VOID *)StartDescTag,sizeof(ADMA_DESC_S)*num);\
        }\
        else\
        {\
            (void)SLAVE_FLUSH_CACHE((BSP_VOID *)StartDescTag,sizeof(ADMA_DESC_S)*(pstAdmahandle->DescCount - StartIdxTag));\
            (void)SLAVE_FLUSH_CACHE((BSP_VOID *)pstAdmahandle->Desc,sizeof(ADMA_DESC_S)*(num + StartIdxTag-pstAdmahandle->DescCount));\
        }\
    }
#if 0
#define ADMA_INVALIDATE_DESC(pstAdmahandle,StartIdxTag,StartDescTag,num,flag)\
    if((flag))\
    {\
        if(StartIdxTag+num<=pstAdmahandle->DescCount)\
        {\
            //cacheInvalidate(DATA_CACHE, (BSP_VOID *)StartDescTag, sizeof(ADMA_DESC_S)*num);\
            __dam_single_cpu_to_dev(DATA_CACHE, (BSP_VOID *)StartDescTag, sizeof(ADMA_DESC_S)*num);\
            CACHE_PIPE_FLUSH();\
        }\
        else\
        {\
            //cacheInvalidate(DATA_CACHE,(BSP_VOID *)StartDescTag,sizeof(ADMA_DESC_S)*(pstAdmahandle->DescCount-StartIdxTag));\
             __dam_single_cpu_to_dev(DATA_CACHE,(BSP_VOID *)StartDescTag,sizeof(ADMA_DESC_S)*(pstAdmahandle->DescCount-StartIdxTag));\
            CACHE_PIPE_FLUSH();\
            //cacheInvalidate(DATA_CACHE,(BSP_VOID *)pstAdmahandle->Desc,sizeof(ADMA_DESC_S)*(num+StartIdxTag-pstAdmahandle->DescCount));\
            __dam_single_cpu_to_dev(DATA_CACHE,(BSP_VOID *)pstAdmahandle->Desc,sizeof(ADMA_DESC_S)*(num+StartIdxTag-pstAdmahandle->DescCount));\
            CACHE_PIPE_FLUSH();\
        }\
    }
#endif

#define ADMA_FLUSH_BUF(buf,len,flag) \
if((flag))\
    {\
       (void)SLAVE_FLUSH_CACHE((BSP_VOID *)(buf),(len));\
    }
           
#define ADMA_INVALIDATE_BUF(buf,len,flag) \
if((flag))\
    {\
        (void)SLAVE_INVALID_CACHE((BSP_VOID *)(buf),(len));\
    }


/**�������ж���**/
//struct work_struct Fstwork;
 

/**************************************************************************
  list ����
**************************************************************************/
#ifndef INLINE
#define INLINE __inline__
#endif

#ifndef LOCAL
#define LOCAL static
#endif
/*
 * Simple doubly linked list implementation.
 *
 * Some of the internal functions ("__xxx") are useful when
 * manipulating whole lists rather than single entries, as
 * sometimes we already know the next/prev entries and we can
 * generate better code by using them directly rather than
 * using the generic single-entry routines.
 */

// struct list_head {
	//struct list_head *next, *prev;
//};

#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define INIT_LIST_HEAD(ptr) do { \
	(ptr)->next = (ptr); (ptr)->prev = (ptr); \
} while (0)

/*
 * Insert a new entry between two known consecutive entries. 
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
//LOCAL INLINE void __list_add(struct list_head * new,
	//struct list_head * prev,
	//struct list_head * next)
//{
	//next->prev = new;
	//new->next = next;
	//new->prev = prev;
	//prev->next = new;
//}

/**
 * list_add_tail - add a new entry
 * @new: new entry to be added
 * @head: list head to add it before
 *
 * Insert a new entry before the specified head.
 * This is useful for implementing queues.
 */
//LOCAL INLINE void list_add_tail(struct list_head *new, struct list_head *head)
//{
//	__list_add(new, head->prev, head);
//}

/*
 * Delete a list entry by making the prev/next entries
 * point to each other.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
//LOCAL INLINE void __list_del(struct list_head * prev,
				//  struct list_head * next)
//{
	//next->prev = prev;
	//prev->next = next;
//}

/**
 * list_del - deletes entry from list.
 * @entry: the element to delete from the list.
 * Note: list_empty on entry does not return true after this, the entry is in an undefined state.
 */
#if 0
LOCAL INLINE void list_del(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
	entry->next = entry->prev = 0;
}
#endif

/**
 * list_del_init - deletes entry from list and reinitialize it.
 * @entry: the element to delete from the list.
 */
//LOCAL INLINE void list_del_init(struct list_head *entry)
//{
//	__list_del(entry->prev, entry->next);
//	INIT_LIST_HEAD(entry); 
//}

/**
 * list_empty - tests whether a list is empty
 * @head: the list to test.
 */
//LOCAL INLINE int list_empty(struct list_head *head)
//{
//	return head->next == head;
//}

/**
 * list_entry - get the struct for this entry
 * @ptr:	the &struct list_head pointer.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_struct within the struct.
 */
#define list_entry(ptr, type, member) \
	((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))

/**
 * list_for_each	-	iterate over a list
 * @pos:	the &struct list_head to use as a loop counter.
 * @head:	the head for your list.
 */
//#define list_for_each(pos, head) \
//	for (pos = (head)->next, prefetch(pos->next); pos != (head); \
 //       	pos = pos->next, prefetch(pos->next))
        	
/**
 * list_for_each_safe	-	iterate over a list safe against removal of list entry
 * @pos:	the &struct list_head to use as a loop counter.
 * @n:		another &struct list_head to use as temporary storage
 * @head:	the head for your list.
 */
//#define list_for_each_safe(pos, n, head) \
//	for (pos = (head)->next, n = pos->next; pos != (head); \
//		pos = n, n = pos->next)

/*define interface*/
BSP_S32 SLAVE_Init();

#ifdef __cplusplus
}
#endif
#endif
