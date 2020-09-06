
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
 
  
#ifndef __RDR_PUB_H__
#define __RDR_PUB_H__

#if !defined(__FASTBOOT__)
#include <linux/module.h>
#endif

#include <osl_types.h>


#define RDR_OK          (0)
#define RDR_ERROR       ((u32)(-1))

#define STR_MODULENAME_MAXLEN		16
#define STR_EXCEPTIONDESC_MAXLEN	48
#define STR_TASKNAME_MAXLEN		    16
#define STR_USERDATA_MAXLEN		    64

enum EDITION_KIND {
    EDITION_USER                        = 1,
    EDITION_INTERNAL_BETA               = 2,
    EDITION_OVERSEA_BETA                = 3,
    EDITION_MAX
};


enum MNTN_CORE{
    MNTN_AP = 0,
    MNTN_CP,
    MNTN_MDMAP,
    MNTN_LPM3,
    MNTN_TEEOS,
    MNTN_MAX
};

#define RDR_CORE(n)         (1 << (n))
enum RDR_CORE_LIST{
    RDR_HIFI    = 0,
    RDR_AP      = RDR_CORE(MNTN_AP),
    RDR_CP      = RDR_CORE(MNTN_CP),
    RDR_MDMAP   = RDR_CORE(MNTN_MDMAP),
    RDR_LPM3    = RDR_CORE(MNTN_LPM3),
    RDR_TEEOS   = RDR_CORE(MNTN_TEEOS),
    RDR_MAX     = RDR_CORE(MNTN_MAX)
};


#define HISI_MNTN_EXC_AP_START              (0x80000000)
#define HISI_MNTN_EXC_AP_END                (0x81FFFFFF)
#define HISI_MNTN_EXC_CP_START              (0)
#define HISI_MNTN_EXC_CP_END                (0)
#define HISI_MNTN_EXC_TEEOS_START           (0x83000000)
#define HISI_MNTN_EXC_TEEOS_END             (0x83FFFFFF)
#define HISI_MNTN_EXC_MDMAP_START           (0)
#define HISI_MNTN_EXC_MDMAP_END             (0)
#define HISI_MNTN_EXC_LPM3_START            (0x85000000)
#define HISI_MNTN_EXC_LPM3_END              (0x85FFFFFF)

#if(FEATURE_ON == MBB_COMMON)
#define RDR_MODEM_CP_WDT_MODID             (0x82000003)  /* CP看门狗异常，保存log，phone和mbb都有 */
#endif
/*
    添加异常类型，请同时修改rdr_exc.c的g_stExcType异常类型表
    */
typedef enum
{
    FAST_POWERUP,               /*产线块式开机*/
    POWERKEY_POWERUP,     /*按键500ms开机*/
    VBUS_POWERUP,               /*插入充电器200ms开机*/
    ALARM_ON_POWERUP,           /*闹铃唤醒开机*/
    POWERHOLD_POWERUP,          /*power hold触发开机*/
    POWERKEY_SHUTDOWN,      /*按键10s关机*/
    POWERHOLD_SHUTDOWN,         /*power hold触发关机*/
    NORMAL_POWERUP_SHUTDOWN,         /*以上为单板上电启动，只记录启动信息，不保存异常文件*/

    RESTART_REBOOT_START = NORMAL_POWERUP_SHUTDOWN,
    POWERKEY_10S_REBOOT = RESTART_REBOOT_START,  /*按键10s重启*/
    RESETKEY_REBOOT,        /*按键重启*/
    SOFT_REBOOT,            /*软件重启*/
    RESTART_REBOOT_END,        /*单板重启，需要判断软件是否发生异常*/

    PMU_EXCEPTION = RESTART_REBOOT_END,/*PMU异常，只记录启动信息，不保存异常文件*/

    /*AP的异常类型 AP_EXC_START~AP_EXC_END */
    AP_EXC_START,
    AP_S_WDT_TIMEOUT,
    AP_S_EXC_PANIC,
    AP_S_EXC_SFTRESET,
    AP_S_EXC_PANIC_INT,
    AP_EXC_END,
    /*LPM3的异常类型 LPM3_EXC_START~LPM3_EXC_END */
    LPM3_EXC_START = AP_EXC_END,
    LPM3_S_EXCEPTION,
    LPM3_S_WDT_TIMEOUT,

    LPM3_EXC_END,

    /*TEEOS的异常类型 TEEOS_EXC_START~TEEOS_EXC_END */
    TEEOS_EXC_START = LPM3_EXC_END,
    TEEOS_S_WDT_TIMEOUT,

    TEEOS_EXC_END,
    /*CP的异常类型 CP_EXC_START~CP_EXC_END */
    /*CP的异常类型要与adrv.h的手机版本保持一致，请谨慎修改*/ 
    
    CP_EXC_START = TEEOS_EXC_END,
    CP_S_MODEMDMSS = 0x2c,
    CP_S_MODEMNOC = 0x2d,
    CP_S_MODEMAP = 0x2e,
    CP_S_EXCEPTION = 0x2f,
    CP_S_RESETFAIL = 0x30,
    CP_S_NORMALRESET = 0x31,
    CP_S_RILD_EXCEPTION = 0x3b,
    CP_S_3RD_EXCEPTION = 0x3c,
    CP_EXC_END,

    RDR_TEST_WDT_TIMEOUT = CP_EXC_END,
    RDR_TEST_PANIC,
    RDR_TEST_DIE,
    RDR_TEST_STACKOVERFLOW,

    UNDEFINE,
}MNTN_EXC_TYPE;


enum REBOOT_PRI {
    RDR_REBOOT_NOW      = 0x01,
    RDR_REBOOT_WAIT,
    RDR_REBOOT_NO,
    RDR_REBOOT_MAX
};

enum PROCESS_PRI {
    RDR_ERR      = 0x01,
    RDR_WARN,
    RDR_OTHER,
    RDR_PPRI_MAX
};

enum REENTRANT {
    RDR_REENTRANT_ALLOW = 0xff00da00,
    RDR_REENTRANT_DISALLOW
};

enum UPLOAD_FLAG {
    RDR_UPLOAD_YES = 0xff00fa00,
    RDR_UPLOAD_NO
};


enum RDR_RETURN {
    RDR_SUCCESSED                   = 0x9f000000,
    RDR_FAILD                       = 0x9f000001,
    RDR_NULLPOINTER                 = 0x9f0000ff
};
#if !defined(__FASTBOOT__)
typedef void (*rdr_e_callback)( u32, void* );
/*
 *   struct list_head   e_list;
 *   u32 modid,			    exception id;
 *		if modid equal 0, will auto generation modid, and return it.
 *   u32 modid_end,		    can register modid region. [modid~modid_end];
		need modid_end >= modid,
 *		if modid_end equal 0, will be register modid only,
		but modid & modid_end cant equal 0 at the same time.
 *   u32 process_priority,	exception process priority
 *   u32 reboot_priority,	exception reboot priority
 *   u64 save_log_mask,		need save log mask
 *   u64 notify_core_mask,	need notify other core mask
 *   u64 reset_core_mask,	need reset other core mask
 *   u64 from_core,		    the core of happen exception
 *   u32 reentrant,		    whether to allow exception reentrant
 *   u32 exce_type,		    the type of exception
 *   char* from_module,		    the module of happen excption
 *   char* desc,		        the desc of happen excption
 *   rdr_e_callback callback,	will be called when excption has processed.
 *   u32 reserve_u32;		reserve u32
 *   void* reserve_p		    reserve void *
 */
struct rdr_exception_info_s {
	struct list_head e_list;
	u32	e_modid;                    /*必填*/
	u32	e_modid_end;                /*必填 e_modid_end>= e_modid*/
	u32	e_process_priority;         /*必填  < RDR_PPRI_MAX*/
	u32	e_reboot_priority;          /*必填 < RDR_REBOOT_MAX*/
	u64	e_notify_core_mask;         /*必填 < RDR_MAX*/
	u64	e_reset_core_mask;          /*必填 < RDR_MAX*/
	u64	e_from_core;                /*必填 < RDR_MAX,枚举值使用rdr_pub.h*/
	u32	e_reentrant;                /*选填，默认不支持重入 */
    u32 e_exce_type;                /*必填 与rdr_exc.h中对应*/
    u32 e_upload_flag;              /*选填，默认不支持upload*/
	u8	e_from_module[STR_MODULENAME_MAXLEN];   /*必填*/
	u8	e_desc[STR_EXCEPTIONDESC_MAXLEN];       /*必填*/
	u32	e_reserve_u32;                  /*保留*/
	void*	e_reserve_p;                 /*保留*/
	rdr_e_callback e_callback;                  /*选填,不填时必须为空*/
};

/*
 * func name: pfn_cb_dump_done
 * func args:
 *    u32   modid
 *      exception id
 *    u64   coreid
 *      which core done
 * return value		null
 */
typedef void (*pfn_cb_dump_done)( u32 modid, u64 coreid);

/*
 * func name: pfn_dump
 * func args:
 *    u32   modid
 *      exception id
 *    u64   coreid
 *      exception core
 *    u32   etype
 *      exception type
 *    char*     logpath
 *      exception log path
 *    pfn_cb_dump_done fndone
 * return       mask bitmap.
 * 注意:回调过程中不允许有阻塞操作
 */
typedef void (*pfn_dump)( u32 modid, u32 etype, u64 coreid,
                char* logpath, pfn_cb_dump_done fndone);
/*
 * func name: pfn_reset
 * func args:
 *    u32   modid
 *      exception id
 *    u32   coreid
 *      exception core
 *    u32   e_type
 *      exception type
 * return value		null
 * 注意:回调过程中不允许有阻塞操作
 */
typedef void (*pfn_reset)( u32 modid, u32 etype, u64 coreid);

struct rdr_module_ops_pub {
    pfn_dump    ops_dump;
    pfn_reset   ops_reset;
};

struct rdr_register_module_result {
    u64     log_addr;
    u32     log_len;
    u64     nve;
};

#ifdef CONFIG_BALONG_RDR
/*
 * func name: rdr_register_exception_type
 * func args:
 *    struct rdr_exception_info_pub* s_e_type
 * return value		e_modid
 *	< 0 error
 *	>=0 success
 */
u32 rdr_register_exception(struct rdr_exception_info_s* e);

/*
 * func name: rdr_register_module_ops
 * func args:
 *   u32 coreid,       core id;
 *      .
 *   struct rdr_module_ops_pub* ops;
 *   struct rdr_register_module_result* retinfo
 * return value		e_modid
 *	< 0 error
 *	>=0 success
 */
int rdr_register_module_ops(
        u64 coreid,
        struct rdr_module_ops_pub* ops,
        struct rdr_register_module_result* retinfo
		);



/*
 * func name: rdr_system_error
 * func args:
 *   u32 modid,			modid( must be registered);
 *   u32 arg1,			arg1;
 *   u32 arg2,			arg2;
 *   char *  data,			short message.
 *   u32 length,		len(IMPORTANT: <=4k)
 * return void
 */
void rdr_system_error(u32 modid, u32 arg1, u32 arg2);
void rdr_fetal_system_error(u32 modid, u32 arg1, u32 arg2);
/*
 * 函数名: bbox_check_edition
 * 函数参数:
 *     void
 * 返回值:
 *     unsigned int:	返回版本信息
 *				0x01        USER
 *				0x02        INTERNAL BETA
 *                         0x03        OVERSEA BETA
 *
 * 该函数会访问用户的data分区，因此依赖于文件系统的正确挂载。
 * 由于没有超时机制，等待文件系统挂载的过程会导致进程进入
 * 不确定时长的睡眠。综上在不能睡眠的场景不能调用该接口。
 */
unsigned int bbox_check_edition(void);
#else
static inline u32 rdr_register_exception(struct rdr_exception_info_s* e){ return 0;}
static inline int rdr_register_module_ops(
        u64 coreid,
        struct rdr_module_ops_pub* ops,
        struct rdr_register_module_result* retinfo
		){ return -1; }
static inline void rdr_system_error(u32 modid, u32 arg1, u32 arg2){}
static inline void rdr_fetal_system_error(u32 modid, u32 arg1, u32 arg2){}
static inline unsigned int bbox_check_edition(void){return EDITION_INTERNAL_BETA;}

#endif


#endif /*__FASTBOOT__*/

#endif/*__RDR_PUB_H__*/



