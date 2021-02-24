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
/*��Э��㲻����ecm ncm ���Զ���ecm �ӿڼ�����������Ҫ��ncm һ�� 
*��ecm �еĶ��嶼�ú��ö��ת����ncm��Ӧ��ֵ*/
#include "mdrv_udi.h"

#include "f_ecm_mbb.h"

#define ECM_NET_USED_NUM                USB_NUM_2
/*����Э��㹲��ncm ����ECM���ж���ȡֵ��Ҫ��ncm  ��ؽṹö��һ��*/
/* ECM�豸����ö��*/
typedef enum tagECM_DEV_TYPE_E
{
    ECM_DEV_DATA_TYPE = NCM_DEV_DATA_TYPE,      /* ����ͨ�����ͣ�PSʹ��*/
    ECM_DEV_CTRL_TYPE = NCM_DEV_CTRL_TYPE,      /* ����ͨ����MSP����AT����ʹ��*/
}ECM_DEV_TYPE_E;

/* 1:ECM ö��ֵ �ṹ�¶��� */
#define ECM_NET_USED_NUM                USB_NUM_2

/* USB NDIS IOCTLö�� */
typedef enum tagECM_IOCTL_CMD_TYPE_E
{
    ECM_IOCTL_NETWORK_CONNECTION_NOTIF      = NCM_IOCTL_NETWORK_CONNECTION_NOTIF,       /* 0x0,NCM�����Ƿ�������*/
    ECM_IOCTL_CONNECTION_SPEED_CHANGE_NOTIF = NCM_IOCTL_CONNECTION_SPEED_CHANGE_NOTIF,  /* 0x1,NCM�豸Э�̵������ٶ�*/
    ECM_IOCTL_SET_PKT_ENCAP_INFO            = NCM_IOCTL_SET_PKT_ENCAP_INFO,             /* 0x2,���ð���װ��ʽ*/
    ECM_IOCTL_REG_UPLINK_RX_FUNC            = NCM_IOCTL_REG_UPLINK_RX_FUNC,             /* 0x3,ע�������հ��ص�����*/
    ECM_IOCTL_REG_FREE_PKT_FUNC             = NCM_IOCTL_REG_FREE_PKT_FUNC,              /* 0x4,ע���ͷŰ���װ�ص�����*/
    ECM_IOCTL_FREE_BUFF                     = NCM_IOCTL_FREE_BUFF,                      /* 0x5,�ͷŵ���buffer*/
    ECM_IOCTL_GET_USED_MAX_BUFF_NUM         = NCM_IOCTL_GET_USED_MAX_BUFF_NUM,          /* 0x6,��ȡ�ϲ�������ռ�õ�ncm buffer����*/
    ECM_IOCTL_GET_DEFAULT_TX_MIN_NUM        = NCM_IOCTL_GET_DEFAULT_TX_MIN_NUM,         /* 0x7,��ȡĬ�Ϸ���������ֵ����������ֵ������һ��NCM����*/
    ECM_IOCTL_GET_DEFAULT_TX_TIMEOUT        = NCM_IOCTL_GET_DEFAULT_TX_TIMEOUT,         /* 0x8,��ȡĬ�Ϸ�����ʱʱ�䣬������ʱ�������һ��NCM����*/
    ECM_IOCTL_GET_DEFAULT_TX_MAX_SIZE       = NCM_IOCTL_GET_DEFAULT_TX_MAX_SIZE,        /* 0x9,��ȡĬ�Ϸ����ֽ���ֵ����������ֵ������һ��NCM����*/
    ECM_IOCTL_SET_TX_MIN_NUM                = NCM_IOCTL_SET_TX_MIN_NUM,                 /* 0xa,���÷���������ֵ����������ֵ������һ��NCM����*/
    ECM_IOCTL_SET_TX_TIMEOUT                = NCM_IOCTL_SET_TX_TIMEOUT,                 /* 0xb,���÷�����ʱʱ�䣬������ʱ�������һ��NCM����*/
    ECM_IOCTL_SET_TX_MAX_SIZE               = NCM_IOCTL_SET_TX_MAX_SIZE,                /* 0xc,�������ֲ���ʹ�á����÷����ֽ���ֵ����������ֵ������һ��NCM����*/
    ECM_IOCTL_GET_RX_BUF_SIZE               = NCM_IOCTL_GET_RX_BUF_SIZE,                /* 0xd,��ȡ�հ�buffer��С*/
    ECM_IOCTL_FLOW_CTRL_NOTIF               = NCM_IOCTL_FLOW_CTRL_NOTIF,                /* 0xe,���ؿ��ƿ���*/
    ECM_IOCTL_REG_AT_PROCESS_FUNC           = NCM_IOCTL_REG_AT_PROCESS_FUNC,            /* 0xf,ע��AT�����ص�����*/
    ECM_IOCTL_AT_RESPONSE                   = NCM_IOCTL_AT_RESPONSE,                    /* 0x10,AT�����Ӧ*/
    ECM_IOCTL_REG_CONNECT_STUS_CHG_FUNC     = NCM_IOCTL_REG_CONNECT_STUS_CHG_FUNC,      /* 0x11,ע������״̬�ı�֪ͨ�ص�����*/
    ECM_IOCTL_SET_PKT_STATICS               = NCM_IOCTL_SET_PKT_STATICS,                /* 0x12,����ͳ����Ϣ*/
    /* ��BSP ������ USB ECM��PS��Ҫ�����ṩECM����״̬��ѯ�ӿ�*/
    ECM_IOCTL_GET_FLOWCTRL_STATUS           = NCM_IOCTL_GET_FLOWCTRL_STATUS,            /* 0x13 ��ѯNCM����״̬*/                
    /*PS��MSP�������ṩ��ѯ��ǰ�������޸����ӿں�ECM ����ʱ�ص���������ECM �豸ID����*/  
    ECM_IOCTL_GET_CUR_TX_MIN_NUM            = NCM_IOCTL_GET_CUR_TX_MIN_NUM,             /* 0x14 ��ȡ��ǰ����������ֵ*/                
    ECM_IOCTL_GET_CUR_TX_TIMEOUT            = NCM_IOCTL_GET_CUR_TX_TIMEOUT,             /* 0x15 ��ȡ��ǰ������ʱʱ��*/                
    ECM_IOCTL_IPV6_DNS_NOTIF                = NCM_IOCTL_IPV6_DNS_NOTIF,                 /* 0x16 IPV6 DNS�����ϱ�*/                
    ECM_IOCTL_SET_IPV6_DNS                  = NCM_IOCTL_SET_IPV6_DNS,                   /* 0x17 ����IPV6 DNS*/                
    ECM_IOCTL_CLEAR_IPV6_DNS                = NCM_IOCTL_CLEAR_IPV6_DNS,                 /* 0x18 ���IPV6 DNS�ڰ�˵Ļ���,param�ڴ�������û�����壬�����ָ�뼴��*/                
    ECM_IOCTL_GET_ECM_STATUS                = NCM_IOCTL_GET_NCM_STATUS,                 /* 0x19 ��ȡNCM����״̬ enable:TRUE(1);disable:FALSE(0) */                
    ECM_IOCTL_SET_ACCUMULATION_TIME         = NCM_IOCTL_SET_ACCUMULATION_TIME,                
    ECM_IOCTL_SET_RX_MIN_NUM                = NCM_IOCTL_SET_RX_MIN_NUM,                 /*0x1B�����հ�������ֵ*/
    ECM_IOCTL_SET_RX_TIMEOUT                = NCM_IOCTL_SET_RX_TIMEOUT,                 /*0x1C�����հ���ʱʱ��*/
    ECM_IOCTL_REG_NDIS_RESP_STATUS_FUNC     = NCM_IOCTL_REG_NDIS_RESP_STATUS_FUNC,      /*0x1DNDISͨ��AT����״̬����ص����� */ 
    ECM_IOCTL_GET_MBIM_FLAG                 = NCM_IOCTL_GET_MBIM_FLAG,                  /*0x1E��ȡ��ǰ�Ƿ���MBIMģʽ */  
    ECM_IOCTL_GET_MNET_FLAG                 = NCM_IOCTL_GET_MNET_FLAG,                  /*0x1F��ȡ��ǰECM �Ƿ��ʵ��*/

}ECM_IOCTL_CMD_TYPE_E;

/* ECM����״̬ö��,ECM_IOCTL_NETWORK_CONNECTION_NOTIF�����ֶ�Ӧ����ö��*/
typedef enum tagECM_IOCTL_CONNECTION_STATUS_E
{
    ECM_IOCTL_CONNECTION_LINKDOWN           = NCM_IOCTL_CONNECTION_LINKDOWN,     /* NCM����Ͽ�����*/
    ECM_IOCTL_CONNECTION_LINKUP             = NCM_IOCTL_CONNECTION_LINKUP        /* ECM��������*/
}ECM_IOCTL_CONNECTION_STATUS_E;

/* ECM����״̬�ṹ, ECM_IOCTL_NETWORK_CONNECTION_NOTIF�����ֶ�Ӧ�����ṹ��*/
typedef NCM_IOCTL_CONNECTION_STATUS_S ECM_IOCTL_CONNECTION_STATUS_S;

/* ECM�����ٶȽṹ,ECM_IOCTL_CONNECTION_SPEED_CHANGE_NOTIF�����ֶ�Ӧ�����ṹ��*/
//typedef NCM_IOCTL_CONNECTION_SPEED_S ECM_IOCTL_CONNECTION_SPEED_S;

/* ����װ�ṹ��,ECM_IOCTL_SET_PKT_ENCAP_INFO�����ֶ�Ӧ�����ṹ��*/
typedef NCM_PKT_ENCAP_INFO_S ECM_PKT_ENCAP_INFO_S;

/* AT����ظ�����ָ�뼰���� ECM_IOCTL_AT_RESPONSE*/
typedef NCM_AT_RSP_S ECM_AT_RSP_S;

typedef NCM_AT_IPV6_DNS_NTF_S ECM_AT_IPV6_DNS_NTF_S;

/* ECM���ؿ���ö��,ECM_IOCTL_NETWORK_CONNECTION_NOTIF�����ֶ�Ӧ����ö��*/
typedef enum tagECM_IOCTL_FLOW_CTRL_E
{
    ECM_IOCTL_FLOW_CTRL_ENABLE              = NCM_IOCTL_FLOW_CTRL_ENABLE,      /* ������*/
    ECM_IOCTL_FLOW_CTRL_DISABLE             = NCM_IOCTL_FLOW_CTRL_DISABLE      /* �ر�����*/
}ECM_IOCTL_FLOW_CTRL_E;

/* ECM����״̬�ı�֪ͨö��,ECM_IOCTL_REG_CONNECT_STUS_CHG_FUNC�����ֶ�Ӧ����ö��*/
typedef enum tagECM_IOCTL_CONNECT_STUS_E
{
    ECM_IOCTL_STUS_CONNECT                  = NCM_IOCTL_STUS_CONNECT,      /* ����*/
    ECM_IOCTL_STUS_BREAK                    = NCM_IOCTL_STUS_BREAK         /* �����Ͽ�,����*/
}ECM_IOCTL_CONNECT_STUS_E;


#define BSP_ECM_IPV6_DNS_LEN     BSP_NCM_IPV6_DNS_LEN
typedef NCM_IPV6DNS_S ECM_IPV6DNS_S;

/*  �´��ϴ������ذ��ĸ���ͳ����Ϣ��
    �����͸�PC�������ȴ�PC�Ķ�ȡ,ECM_IOCTL_SET_PKT_STATICS�����ֶ�Ӧ�����ṹ*/
typedef NCM_IOCTL_PKT_STATISTICS_S ECM_IOCTL_PKT_STATISTICS_S;

typedef struct tagECM_PKT_INFO_S
{
    unsigned int u32PsRcvPktNum;              /* �հ��͵�PS�İ�����*/
    unsigned int u32RcvUnusedNum;             /* �հ�������PSҪ����������*/
    unsigned int u32EcmSendPktNum;            /* ��������*/
}ECM_PKT_INFO_S;

/* 2:ECM���ù��� */

/* ���ջ�������С�����ݺ�������,RNDIS��̬����ҪECM,�������õ�С */
#define ECM_RCV_BUFFER_NUM 8  /* ECM �հ�buffer������ÿ��buffer��С��MAX_RX_TRANSFER_SIZE*/

/* 3:ECM���ù��� */
#define ECM_MAX_TX_TRANSFER_SIZE (42 * 1514)
#define ECM_MAX_RX_TRANSFER_SIZE (42 * 1514)

#define ECM_PACKETS_ACCUMULATION_TIME 2000
#define MAX_ENCAP_CMD_SIZE   2048   /* MSP�ڲ�Լ��AT������󳤶�2048����AT����Э��涨*/

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

/* ECM���ڷ����������ޣ����������޷����ڴ治�㣬
   ��ֹ����sending�����еİ��������࣬���ڴ�غľ�
   ���ݲ���debug�汾160Mbps��������������Ϊ4000
   ����   ʹ������ڴ�
   5000   2012458 
   4000   1624260 
   3500   1430464
   3000   1235702*/
#define ECM_MAX_SEND_PKT_NUM  4000

/* mac��ַ��ʱ�Լ����壬������Ʒ��������*/
#define ECM_EX_MAC_ADDR1  { 0x0C, 0x5B, 0x8F, 0x27, 0x9A, 0x64 }
#define ECM_EX_MAC_ADDR2  { 0x0C, 0x5B, 0x8F, 0x27, 0x9A, 0x65 }
#define ECM_EX_MAC_ADDR3  { 0x0C, 0x5B, 0x8F, 0x27, 0x9A, 0x66 }
#define ECM_EX_MAC_ADDR4  { 0x0C, 0x5B, 0x8F, 0x27, 0x9A, 0x67 }

typedef struct tagECM_SEND_OTHER_DEBUG_S
{    
    u32 u32SendCurCount;              /* ECM��ǰ���ڷ��͵İ�����*/
    u32 u32SendMaxCount;              /* ECM���ڷ����������ޣ����������޷����ڴ治��*/
    u32 u32SendExceedMaxCount;        /* �����������޷��ش���*/
    u32 u32SendNULLCount;             /* �������Ϊ�ջ��豸ID�Ƿ�*/
}ECM_SEND_OTHER_DEBUG_S;

/* USB�ص������ṹ�� */
typedef struct tagUSB_CB_S
{
    USBUpLinkRxFunc rx_cb;           /* �����հ��ص����� */
    USBFreePktEncap pkt_encap_free_cb;      /* ����װ�ͷŻص����� */
    USBNdisAtRecvFunc ndis_at_recv_cb;    /* AT�����ص�*/
    USBNdisStusChgFunc ndis_stat_change_cb;  /* USB����״̬�仯����ص�����*/
    USBNdisRespStatusCB usb_ndis_resp_status_func;  /* NDISͨ��AT����״̬����ص����� */
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
    u32 u32RcvInNum;                 /* �հ������������*/
    u32 u32RcvInSize;                /* �հ����������ֽ��� */
    u32 u32RcvTooLongNum;            /* �հ�������� */
    u32 u32RcvErrPktLen;             /* ������� */
    u32 u32AllocSkbFailNum;          /* ����skbʧ�ܼ��� */
    u32 u32RcvInErrNum;              /* �հ����������쳣�˳�����*/
    u32 u32PsRcvPktNum;              /* �հ��͵�PS�İ�����*/
    u32 u32RcvUnusedNum;             /* �հ�������PSҪ����������*/
    u32 u32ecmSendInNum;             /* ���������������*/    
    u32 u32ecmSendPktNum;            /* ��������*/     
    u32 u32ecmSendNoPktNum;          /* ����ȫ����Ч����*/     
    u32 u32ecmSendOkNum;             /* ���������ɹ�����*/     
    u32 u32ecmSendFailNum;           /* ������������fd��writeʧ�ܴ���*/
    u32 u32ecmSendNoMemNum;          /* �����������벻���ڴ��˳�����*/
    u32 u32ecmSendDevNotOpenNum;     /* ��������ͨ��û�д��˳�����*/
    u32 u32ecmSendDevNotEnableNum;   /* ���������豸û��ʹ���˳�����*/
    u32 u32ecmSendNetNotConnectNum;  /* ������������û����������̬�˳�����*/
    u32 u32ecmSendPktTooLargeNum;    /* ����������������1514�˳�����*/    
    u32 u32ecmSendPktInvalidNum;     /* �����������ݰ���Ч����*/
    u32 u32ecmSendDoneNum;           /* ������ɴ���*/
    u32 u32ecmSendDoneFailNum;       /* �����ص�ʧ�ܴ���*/
    u32 u32ecmAtInNum;              /* AT�����������*/  
    u32 u32ecmAtInOKNum;             /* AT�����ɹ�����*/ 
    u32 u32ecmAtInFailNum;             /* AT����ʧ�ܴ���*/
    u32 u32ecmAtInTimeOutNum;        /* AT������ʱ����*/
    u32 u32ecmAtDoneNum;             /* AT������ɴ���*/
    u32 u32ecmAtCbNum;               /* AT request�ص��������*/
}ECM_DEBUG_INFO_S;

/* ECM˽��������Ϣ */
typedef struct tag_ecm_app_ctx
{    
    u32                         ecm_id;       /* �豸ID��*/
    bool                        bUsed;          /* �Ƿ�ʹ��*/
    bool                        bEnable;        /* �豸�Ƿ�ʹ��*/    
    bool                        chnl_open[ECM_DEV_CTRL_TYPE + 1];
    bool                        bDataConnect;   /* ����ͨ���Ƿ�������*/
    int32_t                     s32atResponseRet;  /* AT����ͽ��*/
    u32                         pkt_encap_info[3]; /* ����װ�ṹ*/
    u32                         u32AccumulationTime;
    u32                         u32FlowCtrlStus;   /* ����״̬*/
    u32                         u32TxCurMinNum;               /* ��ǰ����������ֵ*/
    u32                         u32TxCurTimeout;              /* ��ǰ������ʱʱ��*/
    USB_CB_S                    cbs;   /* ECM��ػص�����*/
    ECM_DEBUG_INFO_S            debug_info;      /* ������Ϣ*/
    ECM_IOCTL_PKT_STATISTICS_S  pkt_stats; /* ͳ����Ϣ*/
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
  ��������
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
