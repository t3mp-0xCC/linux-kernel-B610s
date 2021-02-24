#ifndef CMUX_UDI_AL_H
#define CMUX_UDI_AL_H

/******************************************************************************
  Copyright ?, 1988-2014, Huawei Tech. Co., Ltd.
  File name:    cmux_udi_al.h
  Author:  shiyanan     Version:  1.00      Date: 20140805
  Description:    cmux lib 适配Balong V3R3平台的UDI机制而写的适配文件。
  对应的头文件。
  Others:         NONE
******************************************************************************/


#include <linux/list.h>
#include <osl_sem.h>
#include "cmux_cmd.h"
#include "mdrv_hsuart.h"
#include "bsp_softtimer.h"

#if ( FEATURE_ON == MBB_HSUART_CMUX )
#if (FEATURE_ON == MBB_WPG_ECALL)
#define MAX_CMUX_VPORT_NO  (5)
#else
#define MAX_CMUX_VPORT_NO  (4)
#endif/*MBB_WPG_ECALL*/
/* buffer define */
#define CMUX_READ_BUF_SIZE   (1536)
#define CMUX_READ_BUF_NUM   (16)
#define CMUX_WRITE_BUF_NUM (64)
#define CMUX_SW_TIMER_LENGTH (900)

/* MODEM 管脚信号结构体定义*/
typedef struct tagCMUX_MODEM_MSC_STRU
{
    unsigned int OP_Dtr     :    1;          /*DTR CHANGE FLAG*/
    unsigned int OP_Dsr     :    1;          /*DSR CHANGE FLAG*/
    unsigned int OP_Cts     :    1;          /*CTSCHANGE FLAG*/
    unsigned int OP_Rts     :    1;          /*RTS CHANGE FLAG*/
    unsigned int OP_Ri      :    1;          /*RI CHANGE FLAG*/
    unsigned int OP_Dcd     :    1;          /*DCD CHANGE FLAG*/
    unsigned int OP_Fc      :    1;          /*FC CHANGE FLAG*/
    unsigned int OP_Brk     :    1;          /*BRK CHANGE FLAG*/
    unsigned int OP_Spare   :    24;         /*reserve*/
    unsigned char   ucDtr;                   /*DTR  VALUE*/
    unsigned char   ucDsr;                   /*DSR  VALUE*/
    unsigned char   ucCts;                   /*DTS VALUE*/
    unsigned char   ucRts;                   /*RTS  VALUE*/
    unsigned char   ucRi;                    /*RI VALUE*/
    unsigned char   ucDcd;                   /*DCD  VALUE*/
    unsigned char   ucFc;                    /*FC  VALUE*/
    unsigned char   ucBrk;                   /*BRK  VALUE*/
    unsigned char   ucBrkLen;                /*BRKLEN VALUE*/
} cmux_modem_msc_stru;

typedef void (*cmux_free_cb_t)(char* pBuff);
typedef void (*cmux_read_cb_t)(unsigned char ucIndex);
typedef void (*cmux_msc_read_cb_t)(unsigned char ucIndex, void* pModemMsc);
typedef void (*cmux_msc_mode_read_cb_t)(unsigned char ucIndex);
typedef void (*cmux_switch_mode_cb_t)(unsigned char ucIndex);
typedef void (*cmux_close_cb_t)(void);
typedef void (*cmux_snc_mode_cb_t)(unsigned char ucIndex, unsigned char enable);


typedef struct tagCMUX_UDI_CALLBACKS_S
{
    /* General Handlers */
    cmux_free_cb_t free_cb;
    cmux_read_cb_t read_cb;
    cmux_msc_read_cb_t msc_read_cb;
    cmux_msc_mode_read_cb_t msc_mode_read_cb;
    cmux_switch_mode_cb_t switch_mode_cb;
    cmux_snc_mode_cb_t snc_mode_cb;
}cmux_udi_cbs;

typedef enum tagCMUX_MEM_STATE
{
    CMUX_MEM_NOT_IN_LIST = 0,
    CMUX_MEM_IN_FREELIST = 1,
    CMUX_MEM_IN_DONELIST = 2
}cmux_mem_state;

/* CMUX 内存信息 */
typedef struct tagCMUX_MEM_NODE
{
    struct list_head list_node;
    cmux_mem_state state;
    unsigned int valid_size;
    unsigned char *mem_buf;
}cmux_mem_node;

/* CMUX数据队列结构体*/
typedef struct tagCMUX_TRANS_INFO_S
{
    struct spinlock list_lock;
    struct list_head free_list;
    struct list_head done_list;
    cmux_mem_node *pmem_start;                   /* MemNode数组首地址 */
    cmux_mem_node *pmem_end;
    cmux_mem_node *pcur_pos;                   /* 当前使用节点*/
    unsigned int  node_size;
    unsigned int  node_num;
    volatile unsigned int  done_cnt;
    /* statistics section */
    unsigned int total_bytes;                      /* 总计数 */
}cmux_trans_info;

/* cmux stat info */
typedef struct tagCMUX_STAT {
    unsigned int write_not_free_cnt;
    unsigned int lost_data_cnt;
    unsigned int syn_write_cnt;
    unsigned int burst_dma_done;
    unsigned int single_dma_done;
    unsigned int single_time_out;
    unsigned int alloc_mem_fail;
    unsigned int sleep_delay_cnt;
    unsigned int tx_node_empty;
    unsigned int tx_empty_dma;
    unsigned int tx_empty_int;
} cmux_stat;

typedef enum
{
    CMUX_UDI_DLC_CTL = 0,
    CMUX_UDI_DLC_AT  = 1,
    CMUX_UDI_DLC_MDM = 2,
    CMUX_UDI_DLC_EXT = 3,
    CMUX_UDI_DLC_GPS = 4,
#if (FEATURE_ON == MBB_WPG_ECALL)
    CMUX_UDI_DLC_ECL = 5,
#endif/*MBB_WPG_ECALL*/
    CMUX_UDI_DLC_MAX = 0xff,
}CMUX_UDI_DLC_NUM;
typedef enum
{
    NO_CMUX_STAGE,
    CMUX_STAGE_1,
    CMUX_STAGE_2,
    CMUX_STAGE_PLUS,
    CMUX_STAGE_3,
    CMUX_STAGE_MAX,
}cmux_stage;
typedef struct tagCMUX_CTX_S
{  
    UDI_DEVICE_ID_E udi_device_id;
    int init;
    int open;    
    cmux_trans_info read_info;                     /* 读信息*/
    cmux_trans_info write_info;                    /* 写信息*/
    cmux_udi_cbs cbs;
    unsigned char DlcIndex;
    cmux_stat stat;
    volatile cmux_stage stage;
    
    struct softtimer_list cmux_state3_timer;
    struct softtimer_list cmux_state2_timer;
    struct softtimer_list cmux_stateplus_timer;
}cmux_ctx;

/* 异步数据收发结构 */
typedef struct tagAT_CMUX_WR_ASYNC_INFO
{
    char* pVirAddr;
    char* pPhyAddr;
    unsigned int u32Size;
    void* pDrvPriv;
}at_cmux_wr_async_info;

int cmux_udi_init(void);
signed int cmux_udi_ioctl(void *pPrivate, unsigned int u32Cmd, void *pParam);
signed int cmux_udi_open(UDI_OPEN_PARAM_S *param, UDI_HANDLE handle);
signed int cmux_udi_close(void *DevCtx);
void cmux_callback_func(unsigned int cbtype, void* ptr, struct cmux_timer* ct);
void cmux_send_msc_status(unsigned char ucIndex, cmux_modem_msc_stru* pModemMsc);
#endif/*MBB_HSUART_CMUX*/
#endif /* CMUX_UDI_AL_H */
