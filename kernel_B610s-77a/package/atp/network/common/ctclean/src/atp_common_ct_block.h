/******************************************************************************
  *         版权所有 (C), 2001-2020, 华为技术有限公司
*******************************************************************************/


#ifndef __ATP_COMMON_CT_BLOCK_H__
#define __ATP_COMMON_CT_BLOCK_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <net/netfilter/nf_conntrack_tuple.h>
#include "commonctcleanmsgapi.h"

#define ATP_CT_BLOCK_DEBUG

#ifdef ATP_CT_BLOCK_DEBUG
#define COMMON_CTBLOCK_DEBUG(format, args...) {printk("FILE: %s LINE: %d: ", __FILE__, __LINE__); printk(format, ## args); printk("\n");}
#else
#define COMMON_CTBLOCK_DEBUG(format, args...)
#endif


#define ATP_COMMON_CT_BLOCK_PROC       "block_rule"

#define ATP_COMMON_CT_BLOCK_HASH_SIZE  256

#define ATP_COMMON_CT_BLOCK_TIME       (60 * HZ)

//!\brief block规则节点
/*!
 * 	@list: 节点链表
 * 	@tuple: 五元组tuple信息
 *	@add_time: 节点加入时间
 */
typedef struct tagATP_COMMON_CT_BLOCK_NODE
{
    struct list_head list;
    struct nf_conntrack_tuple tuple;
    unsigned long add_time;
} ATP_COMMON_CT_BLOCK_NODE;

//!\brief block规则hash桶
/*!
 * 	@list: 每个hash桶的链表头
 * 	@lock: 每个hash桶的锁
 */
typedef struct tagATP_COMMON_CT_BLOCK_HASH
{
    struct list_head list;
    spinlock_t lock;
} ATP_COMMON_CT_BLOCK_HASH;



void atp_common_ct_block_add(const struct nf_conntrack_tuple* tuple);


void atp_common_ct_block_reset_timer(void);



void atp_common_ct_block_start(void);

#ifdef __cplusplus
}
#endif

#endif

