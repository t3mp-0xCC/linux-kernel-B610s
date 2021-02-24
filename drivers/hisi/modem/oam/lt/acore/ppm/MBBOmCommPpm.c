


/****************************************************************************
  1 ͷ�ļ�����
*****************************************************************************/
#include "MbbPsCsCommon.h" 
#include "OmSocketPpm.h"
#include "OmCommonPpm.h"

#include <linux/if.h>
#include <linux/netdevice.h>
#include <linux/inetdevice.h> 

#include "hisocket.h"


#define  THIS_FILE_ID        PS_FILE_ID_OM_SOCKET_PPM_C
/*****************************************************************************
  2 ȫ�ֱ�������
*****************************************************************************/
extern SOCKET_UDP_INFO_STRU g_stUdpInfo;
extern COMM_SOCKET_CTRL_INFO_STRU g_astSockInfo[3];
/* LAN���豸���� */
#define     LOCAL_LAN_DEV_NAME     "br0"
#define     IP_IS_ZERO   (0)
/* �������ڰ�IP��ַ��ȫ�ֱ��� */
PRIVATE_SYMBOL VOS_UINT32  s_local_bind_ip_addr = 0;

/* �����IP��ַ�Ͷ˿ںŵ��ź��� */
PRIVATE_SYMBOL VOS_SEM  g_stSockOmGetIpSem = 0;

VOS_VOID AT_InitIpSockOmRunSem(VOS_VOID)
{
    (VOS_VOID)VOS_SmCreate("SockGetIp", 0, VOS_SEMA4_PRIOR, &g_stSockOmGetIpSem);
}


VOS_VOID AT_GetIpSuspendSockOmServer(VOS_VOID)
{
    (VOS_VOID)VOS_SmP(g_stSockOmGetIpSem, 0);
}


VOS_VOID AT_GetIpWakeUpSockOmServer(VOS_VOID)
{
    (VOS_VOID)VOS_SmV(g_stSockOmGetIpSem);
}


VOS_VOID PPM_CloseAllActiveSockets(VOS_VOID)
{
    /* kd11��Ʒ�ϣ�Debug�汾��Ҫ��datalock�ſ�������hids/hims, release �汾����������hids/hims
       ����ˣ���release�汾ʱ��û�е���OM�˿ڵ�����Ҳ���ᴴ��OM�˿ڵ��ź������˴�ͨ����Ʒ��
       ��֤�˿ںź��ź�������һ����release�汾�в�����om�˿� */
    /* CPE������̬��Ʒ��Debug/release�汾����������hids��hims */
    VOS_UINT32 ulSocketIndex[] = {SOCKET_AT,SOCKET_OM_CFG};
    VOS_UINT32 i = 0;
    VOS_INT32 sockfd = 0;
    VOS_INT32 sockfd_listener = 0;

    (VOS_VOID)VOS_SmP(g_astSockInfo[SOCKET_OM_IND].SmClose, 0);
    if (VOS_TRUE == g_stUdpInfo.bStatus)
    {
        PPM_DisconnectAllPort(OM_LOGIC_CHANNEL_CNF);
        g_stUdpInfo.bStatus = VOS_FALSE;
    }
    (VOS_VOID)VOS_SmV(g_astSockInfo[SOCKET_OM_IND].SmClose);

    /* �������ж˿� */
    for (i = 0; i < (sizeof(ulSocketIndex) / sizeof(ulSocketIndex[0])); i++)
    {
        /* �жϵ�ǰ�˿���Ϣ����ͳ�ʼֵһ���������� */
        if((VOS_NULL_PTR == g_astSockInfo[ulSocketIndex[i]].SmClose)
            && (SOCK_NULL == g_astSockInfo[ulSocketIndex[i]].socket)
            && (SOCK_NULL == g_astSockInfo[ulSocketIndex[i]].listener))
        {
            continue;
        }

        (VOS_VOID)VOS_SmP(g_astSockInfo[ulSocketIndex[i]].SmClose, 0);
        sockfd = g_astSockInfo[ulSocketIndex[i]].socket;
        sockfd_listener = g_astSockInfo[ulSocketIndex[i]].listener;

        /* �˿ڼ�� */
        if (SOCK_NULL != sockfd)
        {
            g_astSockInfo[ulSocketIndex[i]].socket = SOCK_NULL;
            closesocket(sockfd);
        }

        /* �˿ڼ�� */
        if (SOCK_NULL != sockfd_listener)
        {
            g_astSockInfo[ulSocketIndex[i]].listener = SOCK_NULL;
            closesocket(sockfd_listener);
        }
        
        (VOS_VOID)VOS_SmV(g_astSockInfo[ulSocketIndex[i]].SmClose);
    }
}


PRIVATE_SYMBOL int dial_ipaddr_notifier(struct notifier_block *nb, unsigned long msg, void *data)
{
    struct in_ifaddr *ina = VOS_NULL_PTR;
    
    ina = (struct in_ifaddr*)data;

    /* ������� */
    if ((VOS_NULL_PTR == ina) || (NETDEV_UP != msg))
    {
        return NOTIFY_DONE;
    }

    /* ֻƥ�������豸br0 */
    if (0 != VOS_MemCmp(ina->ifa_label, LOCAL_LAN_DEV_NAME, sizeof(LOCAL_LAN_DEV_NAME)))
    {
        return NOTIFY_DONE;
    }

    if ((0 != ina->ifa_address) && (ina->ifa_address != s_local_bind_ip_addr))
    {
        /* ���λ�ȡLAN��IP��ַ */
        if (IP_IS_ZERO == s_local_bind_ip_addr)
        {
            s_local_bind_ip_addr = ina->ifa_address;
            AT_GetIpWakeUpSockOmServer();
        }
        else  /* IP��ַ�����仯 */
        {
            s_local_bind_ip_addr = ina->ifa_address;
            PPM_CloseAllActiveSockets();
        }
    }
      
    return NOTIFY_DONE;
}


VOS_VOID PPM_RegisterInetAddrNotifier(VOS_VOID)
{
    /* ����֪ͨ���� */
    static struct notifier_block s_diag_net_ipaddr_change_notifier = {
        .notifier_call = dial_ipaddr_notifier,
    };
    s_local_bind_ip_addr = 0;
    /* ע�ᵽ�ں�IP��ַ֪ͨ���� */
    (VOS_VOID)register_inetaddr_notifier(&s_diag_net_ipaddr_change_notifier);
}


VOS_UINT32 PPM_SockOmGetIpaddr(VOS_VOID)
{
    return s_local_bind_ip_addr;
}

