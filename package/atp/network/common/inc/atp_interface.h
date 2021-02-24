
#if !defined(_ATP_INTERFACE_H_)
#define _ATP_INTERFACE_H_

#ifdef MTK_CHIP_NAME
#define ATP_LAN1                            "eth0.2"
#define ATP_LAN2                            "eth0.4"
#define ATP_LAN3                            "eth0.3"
#define ATP_LAN4                            "eth0.5"
#else 

#ifdef SUPPORT_ATP_ETH_PORT_ORDER_REVERSED     
//LAN�ӿں궨��
#define ATP_LAN1                            "eth0.5"
#define ATP_LAN2                            "eth0.4"
#define ATP_LAN3                            "eth0.3"
#define ATP_LAN4                            "eth0.2"
#else
#define ATP_LAN1                            "eth0.2"
#define ATP_LAN2                            "eth0.3"
#define ATP_LAN3                            "eth0.4"
#define ATP_LAN4                            "eth0.5"
#endif

#endif

#define ATP_WLAN1                           "wl0"
#define ATP_WLAN2                           "wl0.1"
#define ATP_WLAN3                           "wl0.2"
#define ATP_WLAN4                           "wl0.3"
#define ATP_WLAN5                           "wl0.4"
#define ATP_WLAN6                           "wl0.5"
#define ATP_WLAN7                           "wl0.6"
#define ATP_WLAN8                           "wl0.7"
#define ETHUP_WAN                           ATP_WANETH_INTERFACE_NAME
#define ATP_DEFAULT_BRIDGE_NAME             "br0"

#define ATP_OPENWIFI_O_BRIDGE_NAME          "br1"
#define ATP_OPENWIFI_X_BRIDGE_NAME          "br2"


#define PPPOX_MAX_ID                        (256)

#ifndef ATP_PCLINT

/* ע�⣬������Ĭ�ϴ��ݽ����������ַ������ᳬ��unsigned int�ķ�Χ */
static inline unsigned int str_to_uint(char *pszNum, int base)
{
    unsigned int uiRlt = 0;
    
    if ((!pszNum) || ('\0' == pszNum[0]))
    {
        return 0;
    }
    
    while ('\0' != *pszNum)
    {
        uiRlt = (uiRlt * base) + (*pszNum - '0');
        pszNum++;
    }

    return uiRlt;
}

/* �жϸ����ӿ��Ƿ�PPPoE�����LAN�ӿ� */
#define IS_PPPOX_LAN_DEVICE(dev_name) \
(('p' == dev_name[0]) &&('p' == dev_name[1]) && (PPPOX_MAX_ID > str_to_uint(&dev_name[3], 10)))

/* �жϸ����Ľӿ��Ƿ�ppp WAN�ӿڶ�����ppp����ӿ� */
#define IS_PPP_WAN_DEVICE(dev_name) \
(('p' == dev_name[0]) && (PPPOX_MAX_ID <= str_to_uint(&dev_name[3], 10)))


/* �жϸ����Ľӿ��Ƿ�IPoE WAN�ӿ� */

#ifdef CONFIG_ATP_MBB_ARCH
#define IS_IP_WAN_DEVICE(dev_name) \
(('e' == dev_name[0]) && ('t' == dev_name[1]) && ('h' == dev_name[2]) \
    && ('0' == dev_name[3])&& ('_' == dev_name[4]))
#else
#define IS_IP_WAN_DEVICE(dev_name) \
('n' == dev_name[0])
#endif

/* �жϸ����Ľӿ��Ƿ�LTE�ӿ� */
#define IS_LTE_WAN_DEVICE(dev_name) \
(('r' == dev_name[0]) && ('m' == dev_name[1]))

/* �жϸ����Ľӿ��Ƿ�WAN�ӿ� */
#define IS_WAN_DEVICE(dev_name) \
(IS_IP_WAN_DEVICE(dev_name) || (IS_PPP_WAN_DEVICE(dev_name)) || (IS_LTE_WAN_DEVICE(dev_name)))

#define IS_RTK_LAN_DEVICE(dev_name) \
(('r' == dev_name[0]) && ('t' == dev_name[1]) && ('k' == dev_name[2]))



#ifdef CONFIG_ATP_MBB_ARCH
#define IS_LAN_ETH_DEVICE(dev_name) \
	((('e' == dev_name[0]) && ('t' == dev_name[1]) && ('h' == dev_name[2]) \
	&& ('0' == dev_name[3])&& ('.' == dev_name[4]))\
	||(('e' == dev_name[0]) && ('t' == dev_name[1]) && ('h' == dev_name[2]) \
	&& ('0' == dev_name[3])&& ('\0' == dev_name[4])))
#else
#define IS_LAN_ETH_DEVICE(dev_name) \
    (('e' == dev_name[0]) && ('t' == dev_name[1]) && ('h' == dev_name[2]) \
    && ('0' == dev_name[3]))
#endif


#define IS_LAN_WLAN_DEVICE(dev_name) \
(('w' == dev_name[0]))

#define IS_LAN_DEV(dev_name) \
(IS_LAN_ETH_DEVICE(dev_name) || IS_LAN_WLAN_DEVICE(dev_name))

#define IS_BRIDGE_DEV(dev_name) \
    ('b' == dev_name[0])

#define IS_GRE_DEV(dev_name) \
    ('g' == dev_name[0])

#if 0
#define IS_WAN_DEVICE(dev_name)\
(('b' != dev_name[0]) && ('w' != dev_name[0])&& ('e' != dev_name[0]) && (!IS_PPPOX_LAN_DEVICE(dev_name)))
#endif

#endif


#endif /* _ATP_INTERFACE_H_ */

