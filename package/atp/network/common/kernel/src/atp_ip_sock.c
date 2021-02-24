
#include <linux/igmp.h>
#include "atp_sipalg_sock.h"

/*Original in dev*/
#define IP_CMSG_ORIGINDEV   128


#ifdef CONFIG_TAG_NOVLAN_IPTV_PACKETS
extern u16 g_Vlan_tci;
#endif




int atp_ip_setsock(struct sock* sk, int optname, char __user* optval, int val)
{
    int err = 0;
    struct inet_sock* inet = inet_sk(sk);

    switch (optname)
    {
        case IP_ORIGINDEV:
            if (val)
            {
                inet->cmsg_flags |= IP_CMSG_ORIGINDEV;
            }
            else
            {
                inet->cmsg_flags &= ~IP_CMSG_ORIGINDEV;
            }
            break;
#if defined(CONFIG_SIPALG_RECOREDDSTIP)
        case SIPALG_DSTIP:
        {
            err = atp_sipalg_setsock(sk, optname, optval);
            break;
        }
#endif


#ifdef CONFIG_TAG_NOVLAN_IPTV_PACKETS
        case ET_IGMP_VLAN:
            if (copy_from_user(&g_Vlan_tci, optval, sizeof(g_Vlan_tci)))
            {
                err = -EFAULT;
            }
            break;
#endif


        default:
            return -ENOPROTOOPT;
    }

    return err;
}


int atp_ip_getsock(struct sock* sk, int optname, char __user* optval, int val)
{
    int ret = 0;

    switch (optname)
    {
#if defined(CONFIG_SIPALG_RECOREDDSTIP)
        case SIPALG_DSTIP:
        {
            ret = atp_sipalg_getsock(sk, optname, optval);
            break;
        }
#endif

        default:
            return -ENOPROTOOPT;
    }

    return ret;
}

EXPORT_SYMBOL(atp_ip_getsock);
EXPORT_SYMBOL(atp_ip_setsock);

