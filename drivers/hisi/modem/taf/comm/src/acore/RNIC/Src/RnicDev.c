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


/******************************************************************************
   1 ͷ�ļ�����
******************************************************************************/
#include "RnicEntity.h"
#include "RnicDebug.h"
#include "RnicCtx.h"
#include "RnicDev.h"

/*****************************************************************************
    Э��ջ��ӡ��㷽ʽ�µ�.C�ļ��궨��
*****************************************************************************/

#define THIS_FILE_ID PS_FILE_ID_RNIC_DEV_C

/******************************************************************************
   2 �ⲿ������������
******************************************************************************/

/******************************************************************************
   3 ˽�ж���
******************************************************************************/

/******************************************************************************
   4 ȫ�ֱ�������
*****************************************************************************/
/* �������Ĳ������о�̬ӳ�� */
    static const struct net_device_ops rnic_ops = {
           .ndo_stop                = RNIC_StopNetCard,
           .ndo_open                = RNIC_OpenNetCard,
           .ndo_start_xmit          = RNIC_StartXmit,
           .ndo_set_mac_address     = RNIC_SetMacAddress,
           .ndo_change_mtu          = RNIC_ChangeMtu,
           .ndo_tx_timeout          = RNIC_ProcTxTimeout,
           .ndo_do_ioctl            = RNIC_Ioctrl,
           .ndo_get_stats           = RNIC_GetNetCardStats,
    };

#define RNIC_DEV_NAME_PREFIX            "eth_x"
const RNIC_NETCARD_ELEMENT_TAB_STRU           g_astRnicManageTbl[RNIC_NET_ID_MAX_NUM] =
{
    {
      "",
      {{0x58, 0x02, 0x03, 0x04, 0x05, 0x06}, {0x00, 0x11, 0x09, 0x64, 0x01, 0x01}, 0x0008, {0, 0}},
      {{0x58, 0x02, 0x03, 0x04, 0x05, 0x06}, {0x00, 0x11, 0x09, 0x64, 0x01, 0x01}, 0xDD86, {0, 0}},
      MODEM_ID_0, RNIC_RMNET_ID_0, {0, 0, 0, 0, 0}},

    { "1",
      {{0x58, 0x02, 0x03, 0x04, 0x05, 0x07}, {0x00, 0x11, 0x09, 0x64, 0x01, 0x02}, 0x0008, {0, 0}},
      {{0x58, 0x02, 0x03, 0x04, 0x05, 0x07}, {0x00, 0x11, 0x09, 0x64, 0x01, 0x02}, 0xDD86, {0, 0}},
      MODEM_ID_0, RNIC_RMNET_ID_1, {0, 0, 0, 0, 0}},

    { "2",
      {{0x58, 0x02, 0x03, 0x04, 0x05, 0x08}, {0x00, 0x11, 0x09, 0x64, 0x01, 0x03}, 0x0008, {0, 0}},
      {{0x58, 0x02, 0x03, 0x04, 0x05, 0x08}, {0x00, 0x11, 0x09, 0x64, 0x01, 0x03}, 0xDD86, {0, 0}},
      MODEM_ID_0, RNIC_RMNET_ID_2, {0, 0, 0, 0, 0}},
    { "3",
      {{0x58, 0x02, 0x03, 0x04, 0x05, 0x09}, {0x00, 0x11, 0x09, 0x64, 0x01, 0x04}, 0x0008, {0, 0}},
      {{0x58, 0x02, 0x03, 0x04, 0x05, 0x09}, {0x00, 0x11, 0x09, 0x64, 0x01, 0x04}, 0xDD86, {0, 0}},
      MODEM_ID_0, RNIC_RMNET_ID_3, {0, 0, 0, 0, 0}},

    { "4",
      {{0x58, 0x02, 0x03, 0x04, 0x05, 0x0a}, {0x00, 0x11, 0x09, 0x64, 0x01, 0x05}, 0x0008, {0, 0}},
      {{0x58, 0x02, 0x03, 0x04, 0x05, 0x0a}, {0x00, 0x11, 0x09, 0x64, 0x01, 0x05}, 0xDD86, {0, 0}},
      MODEM_ID_0, RNIC_RMNET_ID_4, {0, 0, 0, 0, 0}},
    {
      "_ims",
      {{0x58, 0x02, 0x03, 0x04, 0x05, 0x0d}, {0x00, 0x11, 0x09, 0x64, 0x01, 0x08}, 0x0008, {0, 0}},
      {{0x58, 0x02, 0x03, 0x04, 0x05, 0x0d}, {0x00, 0x11, 0x09, 0x64, 0x01, 0x08}, 0xDD86, {0, 0}},
      MODEM_ID_0, RNIC_RMNET_ID_VT, {0, 0, 0, 0, 0}},

};


/******************************************************************************
   5 ����ʵ��
******************************************************************************/

VOS_INT RNIC_StopNetCard(
    struct net_device                  *pstNetDev
)
{
    RNIC_NETCARD_DEV_INFO_STRU         *pstPriv = VOS_NULL_PTR;

    /* ��ȡ����˽������ */
    pstPriv = (RNIC_NETCARD_DEV_INFO_STRU *)netdev_priv(pstNetDev);

    /* ʹ��netif_stop_queue()ֹͣ�����������շ� */
    netif_stop_queue(pstNetDev);

    /* ��˽�������е�����״̬��־��Ϊ�ر� */
    pstPriv->enStatus = RNIC_NETCARD_STATUS_CLOSED;

    return 0;
}


VOS_INT RNIC_OpenNetCard(
    struct net_device                  *pstNetDev
)
{
    RNIC_NETCARD_DEV_INFO_STRU         *pstPriv = VOS_NULL_PTR;

    /* ��ȡ����˽������ */
    pstPriv = (RNIC_NETCARD_DEV_INFO_STRU *)netdev_priv(pstNetDev);

    /* ���������������� */
    netif_start_queue(pstNetDev);

    /* ��˽�������е�����״̬��־��Ϊ�� */
    pstPriv->enStatus = RNIC_NETCARD_STATUS_OPENED;

    return 0;
}


netdev_tx_t RNIC_StartXmit(
    struct sk_buff                     *pstSkb,
    struct net_device                  *pstNetDev
)
{
    RNIC_SPEC_CTX_STRU                 *pstNetCntxt = VOS_NULL_PTR;
    RNIC_NETCARD_DEV_INFO_STRU         *pstPriv     = VOS_NULL_PTR;

    pstPriv     = (RNIC_NETCARD_DEV_INFO_STRU *)netdev_priv(pstNetDev);

    pstNetCntxt = RNIC_GetNetCntxtByRmNetId(pstPriv->enRmNetId);
    if (VOS_NULL_PTR == pstNetCntxt)
    {
        IMM_ZcFree(pstSkb);
        RNIC_DBG_NET_TX_RMNETID_ERR_NUM(1);
        return NETDEV_TX_OK;
    }

    /* ��MODEM���ͷַ��������� */
    RNIC_ProcessTxDataByModemType(pstNetCntxt, pstSkb);

    return NETDEV_TX_OK;
}


VOS_INT RNIC_SetMacAddress(
    struct net_device                  *pstNetDev,
    VOS_VOID                           *pMacAddr
)
{
    struct sockaddr                    *pstAddr;
    VOS_INT                             lValid;

    /* ����ת�� */
    pstAddr = (struct sockaddr *)pMacAddr;

    /* MAC��ַΪ��Чֵ */
    lValid = is_valid_ether_addr((VOS_UINT8 *)pstAddr->sa_data);
    if (VOS_FALSE == lValid)
    {
        RNIC_DEV_ERR_PRINTK("RNIC_SetMacAddress: Ether addr is invalid!");
        return -EINVAL;
    }

    /* ��MAC��ַ��ֵ������ */
    memcpy(pstNetDev->dev_addr, pstAddr->sa_data, pstNetDev->addr_len);

    return 0;
}


VOS_INT RNIC_ChangeMtu(
    struct net_device                  *pstNetDev,
    VOS_INT                             lNewMtu
)
{
    /* new_mtu���Ȳ�����Ч��Χ�� */
    if (lNewMtu > RNIC_MAX_PACKET)
    {
        RNIC_DEV_ERR_PRINTK("RNIC_ChangeMtu: Mtu out of range!");
        return -EINVAL;
    }

    /* ����mtu��ɸ�ֵ */
    pstNetDev->mtu = (VOS_UINT)lNewMtu;

    return 0;
}


VOS_VOID RNIC_ProcTxTimeout(
    struct net_device                  *pstNetDev
)
{
    return;
}


VOS_INT RNIC_Ioctrl(
    struct net_device                  *pstNetDev,
    struct ifreq                       *pstIfr,
    VOS_INT                             lCmd
)
{
    return -EOPNOTSUPP;
}


struct net_device_stats *RNIC_GetNetCardStats(
    struct net_device                  *pstNetDev
)

{
    RNIC_NETCARD_DEV_INFO_STRU         *pstPriv = VOS_NULL_PTR;

    /* ��ȡ˽������ */
    pstPriv = (RNIC_NETCARD_DEV_INFO_STRU *)netdev_priv(pstNetDev);

    /* ���豸��ͳ����Ϣ���� */
    return &pstPriv->stStats;
}


VOS_VOID RNIC_DeinitNetCard(
    struct net_device                  *pstNetDev
)
{
    RNIC_NETCARD_DEV_INFO_STRU         *pstPriv     = VOS_NULL_PTR;
    RNIC_SPEC_CTX_STRU                 *pstNetCntxt = VOS_NULL_PTR;

    /* ��ȡ����˽������ */
    pstPriv     = (RNIC_NETCARD_DEV_INFO_STRU *)netdev_priv(pstNetDev);
    pstNetCntxt = RNIC_GET_SPEC_NET_CTX(pstPriv->enRmNetId);

    /* �ر��ز� */
    netif_carrier_off(pstNetDev);

    /* ֹͣ������������ */
    netif_stop_queue(pstNetDev);

    /* ȥע�������豸 */
    unregister_netdev(pstNetDev);

    /* �ͷ������豸 */
    free_netdev(pstNetDev);

    /* ��������豸��Ϣ */
    pstNetCntxt->pstPriv = VOS_NULL_PTR;

    return;
}


VOS_INT __init RNIC_InitNetCard(VOS_VOID)
{
    struct net_device                  *pstDev      = VOS_NULL_PTR;
    RNIC_NETCARD_DEV_INFO_STRU         *pstPriv     = VOS_NULL_PTR;
    RNIC_SPEC_CTX_STRU                 *pstNetCntxt = VOS_NULL_PTR;
    VOS_INT                             lRegRst;
    VOS_UINT8                           ucIndex;

    VOS_UINT8                           aucAddr[RNIC_NET_ID_MAX_NUM][RNIC_MAC_ADDR_LEN] = {
                                                {0x58,0x02,0x03,0x04,0x05,0x06},
                                                {0x58,0x02,0x03,0x04,0x05,0x07},
                                                {0x58,0x02,0x03,0x04,0x05,0x08},
                                                {0x58,0x02,0x03,0x04,0x05,0x09},
                                                {0x58,0x02,0x03,0x04,0x05,0x0a},
                                                {0x58,0x02,0x03,0x04,0x05,0x0b}
                                               };

    printk("RNIC_InitNetCard: entry, %u", VOS_GetSlice());

    for (ucIndex = 0 ; ucIndex < RNIC_NET_ID_MAX_NUM ; ucIndex++)
    {
        /* ��ʼ������˽����Ϣ */
        pstNetCntxt = RNIC_GET_SPEC_NET_CTX(ucIndex);
        pstNetCntxt->pstPriv = VOS_NULL_PTR;

        /* ���������豸netdev��˽������ */
        /* alloc_etherdev����ӿڷ�����net_device�� RNIC_NETCARD_DEV_INFO_STRU�����ӿڵ��ڴ�
           RNIC_NETCARD_DEV_INFO_STRU�ṹ�ҽ���net_device�ĺ��� */
        pstDev = alloc_etherdev(sizeof(RNIC_NETCARD_DEV_INFO_STRU));
        if (VOS_NULL_PTR == pstDev)
        {
            RNIC_DEV_ERR_PRINTK("RNIC_InitNetCard: net ptr is Null!");
            continue;
        }

        /* ���豸����MAC��ַ,�Ժ���Ҫ��̶�ֵ */
        memcpy(pstDev->dev_addr, RNIC_GET_DST_MAC_ADDR(ucIndex), RNIC_MAC_ADDR_LEN);

        /* ����Ĭ�ϵ�MTUֵ */
        pstDev->mtu = RNIC_DEFAULT_MTU;

        snprintf(pstDev->name, sizeof(pstDev->name),
            "%s%s",
            RNIC_DEV_NAME_PREFIX, g_astRnicManageTbl[ucIndex].pucRnicNetCardName);

        /* �����뵽��net_device�ṹ���豸ָ��ĺ�������и�ֵ */
        pstDev->netdev_ops = &rnic_ops;

        /* ����˽�����ݳ�ʼ�� */
        pstPriv = (RNIC_NETCARD_DEV_INFO_STRU *)netdev_priv(pstDev);
        pstPriv->pstDev    = pstDev;
        pstPriv->enRmNetId = ucIndex;

        /* �ر��ز� */
        netif_carrier_off(pstDev);

        /* ֹͣ������������ */
        netif_stop_queue(pstDev);

        /* ����register_netdev������Netcard����ע�� */
        lRegRst = register_netdev(pstDev);
        if (VOS_OK != lRegRst)
        {
            /* ע��ʧ���ͷŴ�����RNIC�豸 */
            RNIC_DEV_ERR_PRINTK("RNIC_InitNetCard:Register netdev failed!");
            free_netdev(pstDev);
            continue;
        }

        /* ��������˽������ */
        pstNetCntxt->pstPriv = pstPriv;

        /* ���ز� */
        netif_carrier_on(pstDev);

        /* ���������������� */
        netif_start_queue(pstDev);
    }

    printk("RNIC_InitNetCard: eixt, %u", VOS_GetSlice());

    return 0;
}






