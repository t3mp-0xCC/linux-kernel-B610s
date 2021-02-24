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


#ifndef __NCM_BALONG_H__
#define __NCM_BALONG_H__

#include <linux/types.h>
#include <linux/skbuff.h>
#include "usb_platform_comm.h"
#include "bsp_ncm.h"
#include "f_ncm_mbb.h"
#include "mdrv_udi.h"

/* 1:RNDIS���ù��� */
#define RNDIS_MAX_TX_TRANSFER_SIZE (45*1514)
#define RNDIS_MAX_RX_TRANSFER_SIZE (45*1514)

/* ������ͽ��ջ�������С */
//#define RNDIS_MULTI_BFR_SIZE       (1)

/* 2:NCM���ù��� */
#define NCM_MAX_TX_TRANSFER_SIZE        (256*1024)
#define NCM_MAX_RX_TRANSFER_SIZE        (32*1024)
#define NCM_CACHE_LINE_SIZE             (32)
#define NCM_RX_BUF_RESERVED_LEN         (NCM_CACHE_LINE_SIZE)
#define LINUX_NCM_MAX_TX_TRANSFER_SIZE  (4*1024)

/* ���ջ�������С�����ݺ�������,RNDIS��̬����ҪNCM,�������õ�С */
#define NCM_RCV_BUFFER_NUM 8  /* NCM �հ�buffer������ÿ��buffer��С��MAX_RX_TRANSFER_SIZE*/

/* 3:ECM���ù��� */
#define ECM_MAX_TX_TRANSFER_SIZE (42 * 1514)
#define ECM_MAX_RX_TRANSFER_SIZE (42 * 1514)

#define NCM_PACKETS_ACCUMULATION_TIME 2000
#define NCM_PACKET_NUM 4
#define MAX_ENCAP_CMD_SIZE   2048   /* MSP�ڲ�Լ��AT������󳤶�2048����AT����Э��涨*/

#define MAX_SEG_SIZE 1514
#define MIN_SEG_SIZE 14
#define NCM_ALIGNMENT 4
#define MIN_NCM_TX 1
#define MIN_NCM_TX_SPEED_FULL 25
#define MAX_NCM_TX 64
#define MAX_HOST_OUT_TIMEOUT 20
#define NCM_TX_TIMEOUT 10
#define FIFO_SIZE  1024
#define IP_HEADER_SIZE 44

#define NCM_MAX_RATE   (1*1000*1000*1000)
#define NCM_MIN_RATE   (1*1000)

#define NCM_AT_ANSWER_LEN  (MAX_ENCAP_CMD_SIZE)
#define NCM_AT_RESPONSE_TIMEOUT   200

/* NCM���ڷ����������ޣ����������޷����ڴ治�㣬
   ��ֹ����sending�����еİ��������࣬���ڴ�غľ�
   ���ݲ���debug�汾160Mbps��������������Ϊ4000
   ����   ʹ������ڴ�
   5000   2012458
   4000   1624260
   3500   1430464
   3000   1235702*/
#define NCM_MAX_SEND_PKT_NUM  4000

/* mac��ַ��ʱ�Լ����壬������Ʒ��������*/
#define NCM_EX_MAC_ADDR1  { 0x0C, 0x5B, 0x8F, 0x27, 0x9A, 0x64 }
#define NCM_EX_MAC_ADDR2  { 0x0C, 0x5B, 0x8F, 0x27, 0x9A, 0x65 }
#define NCM_EX_MAC_ADDR3  { 0x0C, 0x5B, 0x8F, 0x27, 0x9A, 0x66 }
#define NCM_EX_MAC_ADDR4  { 0x0C, 0x5B, 0x8F, 0x27, 0x9A, 0x67 }

/* USB�ص������ṹ�� */
typedef struct tagUSB_CB_S
{
    USBUpLinkRxFunc rx_cb;           /* �����հ��ص����� */
    USBFreePktEncap pkt_encap_free_cb;      /* ����װ�ͷŻص����� */
    USBNdisAtRecvFunc ndis_at_recv_cb;    /* AT�����ص�*/
    USBNdisStusChgFunc ndis_stat_change_cb;  /* USB����״̬�仯����ص�����*/
    USBNdisRespStatusCB usb_ndis_resp_status_func;  /* NDISͨ��AT����״̬����ص����� */
} USB_CB_S;

typedef enum tagNCM_RCV_BUF_STAT_E
{
    NCM_RCV_BUF_FREE,
    NCM_RCV_BUF_DONE,
    NCM_RCV_BUF_PROCESS,
    NCM_RCV_BUF_OUT
} NCM_RCV_BUF_STAT_E;

typedef struct tagNCM_DEBUG_INFO_S
{
    u32 u32RcvInNum;                 /* �հ������������*/
    u32 u32RcvInSize;                /* �հ����������ֽ��� */
    u32 u32RcvTooLongNum;            /* �հ�������� */
    u32 u32RcvErrPktLen;             /* ������� */
    u32 u32AllocSkbFailNum;          /* ����skbʧ�ܼ��� */
    u32 u32RcvInErrNum;              /* �հ����������쳣�˳�����*/
    u32 u32PsRcvPktNum;              /* �հ��͵�PS�İ�����*/
    u32 u32RcvUnusedNum;             /* �հ�������PSҪ����������*/
    u32 u32NcmSendInNum;             /* ���������������*/
    u32 u32NcmSendPktNum;            /* ��������*/
    u32 u32NcmSendNoPktNum;          /* ����ȫ����Ч����*/
    u32 u32NcmSendOkNum;             /* ���������ɹ�����*/
    u32 u32NcmSendFailNum;           /* ������������fd��writeʧ�ܴ���*/
    u32 u32NcmSendNoMemNum;          /* �����������벻���ڴ��˳�����*/
    u32 u32NcmSendDevNotOpenNum;     /* ��������ͨ��û�д��˳�����*/
    u32 u32NcmSendDevNotEnableNum;   /* ���������豸û��ʹ���˳�����*/
    u32 u32NcmSendNetNotConnectNum;  /* ������������û����������̬�˳�����*/
    u32 u32NcmSendPktTooLargeNum;    /* ����������������1514�˳�����*/
    u32 u32NcmSendPktInvalidNum;     /* �����������ݰ���Ч����*/
    u32 u32NcmSendDoneNum;           /* ������ɴ���*/
    u32 u32NcmSendDoneFailNum;       /* �����ص�ʧ�ܴ���*/
    u32 u32NcmAtInNum;              /* AT�����������*/
    u32 u32NcmAtInOKNum;             /* AT�����ɹ�����*/
    u32 u32NcmAtInFailNum;             /* AT����ʧ�ܴ���*/
    u32 u32NcmAtInTimeOutNum;        /* AT������ʱ����*/
    u32 u32NcmAtDoneNum;             /* AT������ɴ���*/
    u32 u32NcmAtCbNum;               /* AT request�ص��������*/
} NCM_DEBUG_INFO_S;

/* NCM˽��������Ϣ */
typedef struct tag_ncm_app_ctx
{
    u32                         ncm_id;       /* �豸ID��*/
    bool                        bUsed;          /* �Ƿ�ʹ��*/
    bool                        bEnable;        /* �豸�Ƿ�ʹ��*/
    bool                        chnl_open[NCM_DEV_CTRL_TYPE + 1];
    bool                        bDataConnect;   /* ����ͨ���Ƿ�������*/
    int32_t                     s32atResponseRet;  /* AT����ͽ��*/
    u32                         pkt_encap_info[3]; /* ����װ�ṹ*/
    u32                         u32AccumulationTime;
    u32                         u32FlowCtrlStus;   /* ����״̬*/
    u32                         u32TxCurMinNum;               /* ��ǰ����������ֵ*/
    u32                         u32TxCurTimeout;              /* ��ǰ������ʱʱ��*/
    USB_CB_S                    cbs;   /* NCM��ػص�����*/
    NCM_DEBUG_INFO_S            debug_info;      /* ������Ϣ*/
    NCM_IOCTL_PKT_STATISTICS_S  pkt_stats; /* ͳ����Ϣ*/
    u8                          au8Ipv6Dns[BSP_NCM_IPV6_DNS_LEN];
    u32                         u32Ipv6DnsLen;
    ncm_vendor_ctx_t*            vendor_ctx;
} ncm_app_ctx_t;

typedef struct tagNCM_SEND_OTHER_DEBUG_S
{
    u32 u32SendCurCount;              /* NCM��ǰ���ڷ��͵İ�����*/
    u32 u32SendMaxCount;              /* NCM���ڷ����������ޣ����������޷����ڴ治��*/
    u32 u32SendExceedMaxCount;        /* �����������޷��ش���*/
    u32 u32SendNULLCount;             /* �������Ϊ�ջ��豸ID�Ƿ�*/
} NCM_SEND_OTHER_DEBUG_S;

#define USB_GET_BUF(s32NcmId,PktEncap)   \
    (*(u32 *)((u32)PktEncap + ncm_get_ctx(s32NcmId)->pkt_encap_info[0]))
#define USB_GET_LEN(s32NcmId,PktEncap)   \
    (*(u32 *)((u32)PktEncap + ncm_get_ctx(s32NcmId)->pkt_encap_info[1]))
#define USB_GET_NEXT(s32NcmId,PktEncap)  \
    (*(u32 *)((u32)PktEncap + ncm_get_ctx(s32NcmId)->pkt_encap_info[2]))

s32 ncm_vendor_init(ncm_vendor_ctx_t* vendor_ctx);
s32 ncm_vendor_uninit(ncm_vendor_ctx_t* vendor_ctx);

USB_INT32 ncm_vendor_notify(USB_VOID* app_ncm, USB_UINT32 cmd, USB_VOID* param);

s32 ncm_vendor_encap_cmd(ncm_app_ctx_t* app_ctx, u8* command, u32 size);

s32 ncm_mbim_send_encap_cmd(void* ctx , unsigned char* data, unsigned int size);

void ncm_mbim_send_encap_rsp_done(int status);

USB_INT32 ncm_vendor_add_response(USB_VOID* app_ncm, USB_UINT8* buf, USB_UINT32 len);
int ncm_bind_func(void* app_ctx);
void ncm_unbind_func(void* app_ctx);
s32 ncm_vendor_open(NCM_DEV_TYPE_E dev_type, u32 dev_id);
s32 ncm_vendor_close(NCM_DEV_TYPE_E dev_type, u32 dev_id);
s32 ncm_vendor_write(u32 dev_id, void* pkt_encap, void* net_priv);
s32 ncm_vendor_ioctl(u32 dev_id, NCM_IOCTL_CMD_TYPE_E cmd, void* param);

extern s32 ecm_vendor_open(NCM_DEV_TYPE_E dev_type, u32 dev_id);
extern s32 ecm_vendor_close(NCM_DEV_TYPE_E dev_type, u32 dev_id);
extern s32 ecm_vendor_write(u32 dev_id, void* pkt_encap, void* net_priv);
extern s32 ecm_vendor_ioctl(u32 dev_id, NCM_IOCTL_CMD_TYPE_E cmd, void* param);

USB_VOID ndis_set_ncm(USB_VOID);
USB_VOID ndis_set_ecm(USB_VOID);

#endif /* __NCM_BALONG_H__ */
