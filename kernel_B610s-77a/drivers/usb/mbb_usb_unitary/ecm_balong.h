/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2013-2015. All rights reserved.
 *
 * mobile@huawei.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#ifndef __ECM_BALONG_H__
#define __ECM_BALONG_H__

#include <linux/types.h>
#include <linux/skbuff.h>
#include "usb_platform_comm.h"
#include "drv_comm.h"
/*在协议层不区分ecm ncm 所以对外ecm 接口及常量定义需要与ncm 一致 
*固ecm 中的定义都用宏或枚举转换到ncm对应的值*/
#include "mdrv_udi.h"

#include "f_ecm_mbb.h"

#define ECM_NET_USED_NUM                USB_NUM_2
/*由于协议层共用ncm 所以ECM所有定义取值需要与ncm  相关结构枚举一致*/
/* ECM设备类型枚举*/
typedef enum tagECM_DEV_TYPE_E
{
    ECM_DEV_DATA_TYPE = NCM_DEV_DATA_TYPE,      /* 数据通道类型，PS使用*/
    ECM_DEV_CTRL_TYPE = NCM_DEV_CTRL_TYPE,      /* 控制通道，MSP传输AT命令使用*/
}ECM_DEV_TYPE_E;

/* 1:ECM 枚举值 结构下定义 */
#define ECM_NET_USED_NUM                USB_NUM_2

/* USB NDIS IOCTL枚举 */
typedef enum tagECM_IOCTL_CMD_TYPE_E
{
    ECM_IOCTL_NETWORK_CONNECTION_NOTIF      = NCM_IOCTL_NETWORK_CONNECTION_NOTIF,       /* 0x0,NCM网络是否连接上*/
    ECM_IOCTL_CONNECTION_SPEED_CHANGE_NOTIF = NCM_IOCTL_CONNECTION_SPEED_CHANGE_NOTIF,  /* 0x1,NCM设备协商的网卡速度*/
    ECM_IOCTL_SET_PKT_ENCAP_INFO            = NCM_IOCTL_SET_PKT_ENCAP_INFO,             /* 0x2,设置包封装格式*/
    ECM_IOCTL_REG_UPLINK_RX_FUNC            = NCM_IOCTL_REG_UPLINK_RX_FUNC,             /* 0x3,注册上行收包回调函数*/
    ECM_IOCTL_REG_FREE_PKT_FUNC             = NCM_IOCTL_REG_FREE_PKT_FUNC,              /* 0x4,注册释放包封装回调函数*/
    ECM_IOCTL_FREE_BUFF                     = NCM_IOCTL_FREE_BUFF,                      /* 0x5,释放底软buffer*/
    ECM_IOCTL_GET_USED_MAX_BUFF_NUM         = NCM_IOCTL_GET_USED_MAX_BUFF_NUM,          /* 0x6,获取上层可以最多占用的ncm buffer个数*/
    ECM_IOCTL_GET_DEFAULT_TX_MIN_NUM        = NCM_IOCTL_GET_DEFAULT_TX_MIN_NUM,         /* 0x7,获取默认发包个数阈值，超过该阈值会启动一次NCM传输*/
    ECM_IOCTL_GET_DEFAULT_TX_TIMEOUT        = NCM_IOCTL_GET_DEFAULT_TX_TIMEOUT,         /* 0x8,获取默认发包超时时间，超过该时间会启动一次NCM传输*/
    ECM_IOCTL_GET_DEFAULT_TX_MAX_SIZE       = NCM_IOCTL_GET_DEFAULT_TX_MAX_SIZE,        /* 0x9,获取默认发包字节阈值，超过该阈值会启动一次NCM传输*/
    ECM_IOCTL_SET_TX_MIN_NUM                = NCM_IOCTL_SET_TX_MIN_NUM,                 /* 0xa,设置发包个数阈值，超过该阈值会启动一次NCM传输*/
    ECM_IOCTL_SET_TX_TIMEOUT                = NCM_IOCTL_SET_TX_TIMEOUT,                 /* 0xb,设置发包超时时间，超过该时间会启动一次NCM传输*/
    ECM_IOCTL_SET_TX_MAX_SIZE               = NCM_IOCTL_SET_TX_MAX_SIZE,                /* 0xc,该命令字不再使用。设置发包字节阈值，超过该阈值会启动一次NCM传输*/
    ECM_IOCTL_GET_RX_BUF_SIZE               = NCM_IOCTL_GET_RX_BUF_SIZE,                /* 0xd,获取收包buffer大小*/
    ECM_IOCTL_FLOW_CTRL_NOTIF               = NCM_IOCTL_FLOW_CTRL_NOTIF,                /* 0xe,流控控制开关*/
    ECM_IOCTL_REG_AT_PROCESS_FUNC           = NCM_IOCTL_REG_AT_PROCESS_FUNC,            /* 0xf,注册AT命令处理回调函数*/
    ECM_IOCTL_AT_RESPONSE                   = NCM_IOCTL_AT_RESPONSE,                    /* 0x10,AT命令回应*/
    ECM_IOCTL_REG_CONNECT_STUS_CHG_FUNC     = NCM_IOCTL_REG_CONNECT_STUS_CHG_FUNC,      /* 0x11,注册网卡状态改变通知回调函数*/
    ECM_IOCTL_SET_PKT_STATICS               = NCM_IOCTL_SET_PKT_STATICS,                /* 0x12,配置统计信息*/
    /* 【BSP 新需求 USB ECM】PS需要底软提供ECM流控状态查询接口*/
    ECM_IOCTL_GET_FLOWCTRL_STATUS           = NCM_IOCTL_GET_FLOWCTRL_STATUS,            /* 0x13 查询NCM流控状态*/                
    /*PS和MSP新需求，提供查询当前发包门限个数接口和ECM 挂起时回调函数增加ECM 设备ID参数*/  
    ECM_IOCTL_GET_CUR_TX_MIN_NUM            = NCM_IOCTL_GET_CUR_TX_MIN_NUM,             /* 0x14 获取当前发包个数阈值*/                
    ECM_IOCTL_GET_CUR_TX_TIMEOUT            = NCM_IOCTL_GET_CUR_TX_TIMEOUT,             /* 0x15 获取当前发包超时时间*/                
    ECM_IOCTL_IPV6_DNS_NOTIF                = NCM_IOCTL_IPV6_DNS_NOTIF,                 /* 0x16 IPV6 DNS主动上报*/                
    ECM_IOCTL_SET_IPV6_DNS                  = NCM_IOCTL_SET_IPV6_DNS,                   /* 0x17 配置IPV6 DNS*/                
    ECM_IOCTL_CLEAR_IPV6_DNS                = NCM_IOCTL_CLEAR_IPV6_DNS,                 /* 0x18 清除IPV6 DNS在板端的缓存,param在此命令字没有意义，不填空指针即可*/                
    ECM_IOCTL_GET_ECM_STATUS                = NCM_IOCTL_GET_NCM_STATUS,                 /* 0x19 获取NCM网卡状态 enable:TRUE(1);disable:FALSE(0) */                
    ECM_IOCTL_SET_ACCUMULATION_TIME         = NCM_IOCTL_SET_ACCUMULATION_TIME,                
    ECM_IOCTL_SET_RX_MIN_NUM                = NCM_IOCTL_SET_RX_MIN_NUM,                 /*0x1B配置收包个数阈值*/
    ECM_IOCTL_SET_RX_TIMEOUT                = NCM_IOCTL_SET_RX_TIMEOUT,                 /*0x1C配置收包超时时间*/
    ECM_IOCTL_REG_NDIS_RESP_STATUS_FUNC     = NCM_IOCTL_REG_NDIS_RESP_STATUS_FUNC,      /*0x1DNDIS通道AT命令状态处理回调函数 */ 
    ECM_IOCTL_GET_MBIM_FLAG                 = NCM_IOCTL_GET_MBIM_FLAG,                  /*0x1E获取当前是否是MBIM模式 */  
    ECM_IOCTL_GET_MNET_FLAG                 = NCM_IOCTL_GET_MNET_FLAG,                  /*0x1F获取当前ECM 是否多实例*/

}ECM_IOCTL_CMD_TYPE_E;

/* ECM连接状态枚举,ECM_IOCTL_NETWORK_CONNECTION_NOTIF命令字对应参数枚举*/
typedef enum tagECM_IOCTL_CONNECTION_STATUS_E
{
    ECM_IOCTL_CONNECTION_LINKDOWN           = NCM_IOCTL_CONNECTION_LINKDOWN,     /* NCM网络断开连接*/
    ECM_IOCTL_CONNECTION_LINKUP             = NCM_IOCTL_CONNECTION_LINKUP        /* ECM网络连接*/
}ECM_IOCTL_CONNECTION_STATUS_E;

/* ECM连接状态结构, ECM_IOCTL_NETWORK_CONNECTION_NOTIF命令字对应参数结构体*/
typedef NCM_IOCTL_CONNECTION_STATUS_S ECM_IOCTL_CONNECTION_STATUS_S;

/* ECM连接速度结构,ECM_IOCTL_CONNECTION_SPEED_CHANGE_NOTIF命令字对应参数结构体*/
//typedef NCM_IOCTL_CONNECTION_SPEED_S ECM_IOCTL_CONNECTION_SPEED_S;

/* 包封装结构体,ECM_IOCTL_SET_PKT_ENCAP_INFO命令字对应参数结构体*/
typedef NCM_PKT_ENCAP_INFO_S ECM_PKT_ENCAP_INFO_S;

/* AT命令回复数据指针及长度 ECM_IOCTL_AT_RESPONSE*/
typedef NCM_AT_RSP_S ECM_AT_RSP_S;

typedef NCM_AT_IPV6_DNS_NTF_S ECM_AT_IPV6_DNS_NTF_S;

/* ECM流控开关枚举,ECM_IOCTL_NETWORK_CONNECTION_NOTIF命令字对应参数枚举*/
typedef enum tagECM_IOCTL_FLOW_CTRL_E
{
    ECM_IOCTL_FLOW_CTRL_ENABLE              = NCM_IOCTL_FLOW_CTRL_ENABLE,      /* 打开流控*/
    ECM_IOCTL_FLOW_CTRL_DISABLE             = NCM_IOCTL_FLOW_CTRL_DISABLE      /* 关闭流控*/
}ECM_IOCTL_FLOW_CTRL_E;

/* ECM网卡状态改变通知枚举,ECM_IOCTL_REG_CONNECT_STUS_CHG_FUNC命令字对应参数枚举*/
typedef enum tagECM_IOCTL_CONNECT_STUS_E
{
    ECM_IOCTL_STUS_CONNECT                  = NCM_IOCTL_STUS_CONNECT,      /* 建链*/
    ECM_IOCTL_STUS_BREAK                    = NCM_IOCTL_STUS_BREAK         /* 网卡断开,断链*/
}ECM_IOCTL_CONNECT_STUS_E;


#define BSP_ECM_IPV6_DNS_LEN     BSP_NCM_IPV6_DNS_LEN
typedef NCM_IPV6DNS_S ECM_IPV6DNS_S;

/*  下传上传、下载包的各种统计信息，
    不发送给PC，被动等待PC的读取,ECM_IOCTL_SET_PKT_STATICS命令字对应参数结构*/
typedef NCM_IOCTL_PKT_STATISTICS_S ECM_IOCTL_PKT_STATISTICS_S;

typedef struct tagECM_PKT_INFO_S
{
    unsigned int u32PsRcvPktNum;              /* 收包送到PS的包个数*/
    unsigned int u32RcvUnusedNum;             /* 收包不符合PS要求丢弃包个数*/
    unsigned int u32EcmSendPktNum;            /* 发包个数*/
}ECM_PKT_INFO_S;

/* 2:ECM配置管理 */

/* 接收缓冲区大小，根据宏来区别,RNDIS形态不需要ECM,所以设置的小 */
#define ECM_RCV_BUFFER_NUM 8  /* ECM 收包buffer个数，每个buffer大小是MAX_RX_TRANSFER_SIZE*/

/* 3:ECM配置管理 */
#define ECM_MAX_TX_TRANSFER_SIZE (42 * 1514)
#define ECM_MAX_RX_TRANSFER_SIZE (42 * 1514)

#define ECM_PACKETS_ACCUMULATION_TIME 2000
#define MAX_ENCAP_CMD_SIZE   2048   /* MSP内部约束AT命令最大长度2048，非AT命令协议规定*/

#define MAX_SEG_SIZE 1514
#define MIN_SEG_SIZE 14
#define ECM_ALIGNMENT 4
#define MIN_ECM_TX 1
#define MIN_ECM_TX_SPEED_FULL 25
#define MAX_ECM_TX 64
#define MAX_HOST_OUT_TIMEOUT 20
#define ECM_TX_TIMEOUT 10
#define FIFO_SIZE  1024

#define ECM_MAX_RATE   (1 * 1000 * 1000 * 1000)
#define ECM_MIN_RATE   (1 * 1000)

#define ECM_AT_ANSWER_LEN  (MAX_ENCAP_CMD_SIZE)
#define ECM_AT_RESPONSE_TIMEOUT   200

/* ECM正在发包个数门限，超过该门限返回内存不足，
   防止正在sending过程中的包个数过多，将内存池耗尽
   根据测试debug版本160Mbps，发包门限配置为4000
   门限   使用最大内存
   5000   2012458 
   4000   1624260 
   3500   1430464
   3000   1235702*/
#define ECM_MAX_SEND_PKT_NUM  4000

/* mac地址临时自己定义，后续产品化再申请*/
#define ECM_EX_MAC_ADDR1  { 0x0C, 0x5B, 0x8F, 0x27, 0x9A, 0x64 }
#define ECM_EX_MAC_ADDR2  { 0x0C, 0x5B, 0x8F, 0x27, 0x9A, 0x65 }
#define ECM_EX_MAC_ADDR3  { 0x0C, 0x5B, 0x8F, 0x27, 0x9A, 0x66 }
#define ECM_EX_MAC_ADDR4  { 0x0C, 0x5B, 0x8F, 0x27, 0x9A, 0x67 }

typedef struct tagECM_SEND_OTHER_DEBUG_S
{    
    u32 u32SendCurCount;              /* ECM当前正在发送的包个数*/
    u32 u32SendMaxCount;              /* ECM正在发包个数门限，超过该门限返回内存不足*/
    u32 u32SendExceedMaxCount;        /* 超过发包门限返回次数*/
    u32 u32SendNULLCount;             /* 传入参数为空或设备ID非法*/
}ECM_SEND_OTHER_DEBUG_S;

/* USB回调函数结构体 */
typedef struct tagUSB_CB_S
{
    USBUpLinkRxFunc rx_cb;           /* 上行收包回调函数 */
    USBFreePktEncap pkt_encap_free_cb;      /* 包封装释放回调函数 */
    USBNdisAtRecvFunc ndis_at_recv_cb;    /* AT命令处理回调*/
    USBNdisStusChgFunc ndis_stat_change_cb;  /* USB网卡状态变化处理回调函数*/
    USBNdisRespStatusCB usb_ndis_resp_status_func;  /* NDIS通道AT命令状态处理回调函数 */
}USB_CB_S;

typedef enum tagECM_RCV_BUF_STAT_E
{
    ECM_RCV_BUF_FREE,
    ECM_RCV_BUF_DONE,
    ECM_RCV_BUF_PROCESS,
    ECM_RCV_BUF_OUT
}ECM_RCV_BUF_STAT_E;

typedef struct tagECM_DEBUG_INFO_S
{    
    u32 u32RcvInNum;                 /* 收包函数进入次数*/
    u32 u32RcvInSize;                /* 收包函数进入字节数 */
    u32 u32RcvTooLongNum;            /* 收包错包计数 */
    u32 u32RcvErrPktLen;             /* 错包包长 */
    u32 u32AllocSkbFailNum;          /* 分配skb失败计数 */
    u32 u32RcvInErrNum;              /* 收包函数进入异常退出次数*/
    u32 u32PsRcvPktNum;              /* 收包送到PS的包个数*/
    u32 u32RcvUnusedNum;             /* 收包不符合PS要求丢弃包个数*/
    u32 u32ecmSendInNum;             /* 发包函数进入次数*/    
    u32 u32ecmSendPktNum;            /* 发包个数*/     
    u32 u32ecmSendNoPktNum;          /* 发包全部无效次数*/     
    u32 u32ecmSendOkNum;             /* 发包函数成功次数*/     
    u32 u32ecmSendFailNum;           /* 发包函数调用fd层write失败次数*/
    u32 u32ecmSendNoMemNum;          /* 发包函数申请不到内存退出次数*/
    u32 u32ecmSendDevNotOpenNum;     /* 发包函数通道没有打开退出次数*/
    u32 u32ecmSendDevNotEnableNum;   /* 发包函数设备没有使能退出次数*/
    u32 u32ecmSendNetNotConnectNum;  /* 发包函数网卡没有配置连接态退出次数*/
    u32 u32ecmSendPktTooLargeNum;    /* 发包函数包长超过1514退出次数*/    
    u32 u32ecmSendPktInvalidNum;     /* 发包函数数据包无效次数*/
    u32 u32ecmSendDoneNum;           /* 发包完成次数*/
    u32 u32ecmSendDoneFailNum;       /* 发包回调失败次数*/
    u32 u32ecmAtInNum;              /* AT发包进入次数*/  
    u32 u32ecmAtInOKNum;             /* AT发包成功次数*/ 
    u32 u32ecmAtInFailNum;             /* AT发包失败次数*/
    u32 u32ecmAtInTimeOutNum;        /* AT发包超时次数*/
    u32 u32ecmAtDoneNum;             /* AT发包完成次数*/
    u32 u32ecmAtCbNum;               /* AT request回调处理次数*/
}ECM_DEBUG_INFO_S;

/* ECM私有数据信息 */
typedef struct tag_ecm_app_ctx
{    
    u32                         ecm_id;       /* 设备ID号*/
    bool                        bUsed;          /* 是否被使用*/
    bool                        bEnable;        /* 设备是否使能*/    
    bool                        chnl_open[ECM_DEV_CTRL_TYPE + 1];
    bool                        bDataConnect;   /* 数据通道是否连接上*/
    int32_t                     s32atResponseRet;  /* AT命令发送结果*/
    u32                         pkt_encap_info[3]; /* 包封装结构*/
    u32                         u32AccumulationTime;
    u32                         u32FlowCtrlStus;   /* 流控状态*/
    u32                         u32TxCurMinNum;               /* 当前发包个数阈值*/
    u32                         u32TxCurTimeout;              /* 当前发包超时时间*/
    USB_CB_S                    cbs;   /* ECM相关回调函数*/
    ECM_DEBUG_INFO_S            debug_info;      /* 调试信息*/
    ECM_IOCTL_PKT_STATISTICS_S  pkt_stats; /* 统计信息*/
    u8                          au8Ipv6Dns[BSP_ECM_IPV6_DNS_LEN];
    u32                         u32Ipv6DnsLen;
    ecm_vendor_ctx_t            *vendor_ctx;
}ecm_app_ctx_t;

/* ecm debug mask */
#define ECM_DBG_MASK_AT       BIT(0)
#define ECM_DBG_MASK_RX       BIT(1)
#define ECM_DBG_MASK_TX       BIT(2)
extern USB_UINT32 gNcmDbgMask;
#define ECM_DBG_MASK_CHECK(_mask) (unlikely(_mask == (gNcmDbgMask & _mask)))
#define ECM_DBG_PRINT_AT(args...) if (ECM_DBG_MASK_CHECK(ECM_DBG_MASK_AT)) {(void)printk(args);}
#define ECM_DBG_PRINT_RX(args...) if (ECM_DBG_MASK_CHECK(ECM_DBG_MASK_RX)) {(void)printk(args);}
#define ECM_DBG_PRINT_TX(args...) if (ECM_DBG_MASK_CHECK(ECM_DBG_MASK_TX)) {(void)printk(args);}

/**************************************************************************
  函数声明
**************************************************************************/
s32 ecm_vendor_init(ecm_vendor_ctx_t *vendor_ctx);
s32 ecm_vendor_uninit(ecm_vendor_ctx_t *vendor_ctx);

USB_INT32 ecm_vendor_notify(USB_VOID *app_ecm, USB_UINT32 cmd, USB_VOID *param);

int ecm_bind_func(void *app_ctx);
void ecm_unbind_func(void *app_ctx);

s32 ecm_vendor_open(ECM_DEV_TYPE_E dev_type, u32 dev_id);
s32 ecm_vendor_close(ECM_DEV_TYPE_E dev_type, u32 dev_id);
s32 ecm_vendor_write(u32 dev_id, void *pkt_encap, void *net_priv);
s32 ecm_vendor_ioctl(u32 dev_id, ECM_IOCTL_CMD_TYPE_E cmd, void * param);

extern USB_VOID ndis_set_ncm(USB_VOID);
extern USB_VOID ndis_set_ecm(USB_VOID);

#endif /* __ECM_BALONG_H__ */
