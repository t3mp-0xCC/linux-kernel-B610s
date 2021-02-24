/******************************************************************************
  *         ��Ȩ���� (C), 2001-2020, ��Ϊ�������޹�˾
*******************************************************************************/


#ifndef __ATP_NTWK_COMMONCTCLEANMSGAPI_H__
#define __ATP_NTWK_COMMONCTCLEANMSGAPI_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __KERNEL__
#include <netinet/in.h>
#endif
#ifdef ATP_CT_CLEAN_DEBUG

#ifndef __KERNEL__

#define NIP6(addr) \
    ntohs((addr).s6_addr16[0]), \
    ntohs((addr).s6_addr16[1]), \
    ntohs((addr).s6_addr16[2]), \
    ntohs((addr).s6_addr16[3]), \
    ntohs((addr).s6_addr16[4]), \
    ntohs((addr).s6_addr16[5]), \
    ntohs((addr).s6_addr16[6]), \
    ntohs((addr).s6_addr16[7])
#define NIP6_FMT "%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x"


#define COMMON_CTCLEAN_DEBUG(format, args...)          printf("<DEBUG> ---> FILE: %s LINE: %d: ", __FILE__, __LINE__); printf(format, ## args); printf("\n")
#else
#define COMMON_CTCLEAN_DEBUG(format, args...)          printk("<DEBUG> ---> FILE: %s LINE: %d: ", __FILE__, __LINE__); printk(format, ## args); printk("\n")
#endif

#else
#define COMMON_CTCLEAN_DEBUG(format, args...)
#endif

#ifdef SUPPORT_ATP_HYBRID_BONDING
#define DSCP_MARK_OFFSET 22/*���ݲ�ͬ��dscp���ϲ�ͬ��markֵ��markֵ��22~27λ��dscpʹ��*/
#endif

#define ATP_COMMON_CT_CLEAN_INTF_LEN 32
#define ATP_COMMON_CT_CLEAN_MAC_LEN  18

#define ATP_COMMON_MAX_IP_PER_MAC_NUM  32

#define ATP_COMMON_CT_CLEAN_ARG_NUM  22

#define ATP_COMMON_CT_CLEAN_BR_MAX   16  /*�ӿڲ���br0:9Ҳ��һ��bridge�豸*/

#define ATP_COMMON_CT_CLEAN_MAX_IPV4 0xFFFFFFFF

#define ATP_COMMON_CT_CLEAN_TRIGGER_CMD  "ping -c 1 127.0.0.1 >/dev/null &"

#define ATP_COMMON_CT_CLEAN_HISI_CMD1    "hi_cli home/cli/debug/app/res/flushlink -v srvname notmc"
#define ATP_COMMON_CT_CLEAN_HISI_CMD2    "hi_cli home/cli/debug/app/res/flushlink -v srvname mc"
#define ATP_COMMON_CT_CLEAN_BCM_CMD      "fcctl flush"
#define ATP_COMMON_CT_CLEAN_BONDING_CMD  "cat proc/net/accel_flush"

typedef enum
{
    ATP_COMMON_CT_CLEAN_ALL       = 0,    /*���е����Ӹ���*/
    ATP_COMMON_CT_CLEAN_NONE      = (1 << 1),   /*�������Ӹ���*/
    ATP_COMMON_CT_CLEAN_BYPASS    = (1 << 2), /*��dsl bypass�����Ӹ���*/
    ATP_COMMON_CT_CLEAN_BONDING   = (1 << 3),/*��bonding�����Ӹ���*/
    ATP_COMMON_CT_CLEAN_ACL       = (1 << 4),    /*ACL����*/
    ATP_COMMON_CT_CLEAN_ROUTE     = (1 << 5),  /*·��ADD*/
    ATP_COMMON_CT_CLEAN_PTTRIGGER = (1 << 6),
    ATP_COMMON_CT_CLEAN_FORWARD   = (1 << 7), /*LAN to WANת��*/
} ATP_COMMON_CT_CLEAN_TYPE;

typedef enum
{
    ATP_COMMON_CT_CLEAN_DIRECTION_ALL,    /*����ע����*/
    ATP_COMMON_CT_CLEAN_DIRECTION_LAN,     /*LAN�෢������Ӹ���*/
    ATP_COMMON_CT_CLEAN_DIRECTION_WAN,     /*WAN�෢������Ӹ���*/
} ATP_COMMON_CT_CLEAN_DIRECTION;


//!\brief ���Ӹ���ɾ������ṹ���û�̬�ں�̬ͨ��
/*!
 * 	@ulSrcIpStart: IPv4Դ��ʼIP������0ʱ����飬������ƥ���ԴIP
 * 	@ulSrcIpEnd: IPv4Դ����IP������0ʱ����飬������ƥ���ԴIP������Mask
 *	@ulSrcMask: IPv4ԴIP��Ӧ���룬����0ʱ����飬�����Դ��ʼIP��ϼ������
 *	@ulDstIpStart: IPv4Ŀ����ʼIP������0ʱ����飬������ƥ���Ŀ��IP
 *	@ulDstIpEnd: IPv4Ŀ�Ľ���IP������0ʱ����飬������ƥ���Ŀ��IP������Mask
 *	@ulDstMask: IPv4Ŀ��IP��Ӧ���룬����0ʱ����飬�����Ŀ��IP��ϼ������
 *    @stSrcIp6Start: IPv6Դ��ʼIP������0ʱ����飬������ƥ���ԴIP
 *    @stSrcIp6End: IPv6Դ����IP������0ʱ����飬������ƥ���ԴIP������Mask
 *	@stSrcMask6: IPv6ԴIP��Ӧ���룬����0ʱ����飬�����ԴIP��ϼ������
 *	@stDstIp6Start: IPv6Ŀ����ʼIP������0ʱ����飬������ƥ���Ŀ��IP
 *	@stDstIp6End: IPv6Ŀ�Ľ����IP������0ʱ����飬������ƥ���Ŀ��IP������Mask
 *	@stDstMask6: IPv6Ŀ��IP��Ӧ���룬����0ʱ����飬�����Ŀ��IP��ϼ������
 *	@usSrcPortStart: Դ�˿ڷ�Χ��ʼֵ������0ʱ�����
 *	@usSrcPortEnd: Դ�˿ڷ�Χ����ֵ������0ʱ�����
 *	@usDstPortStart: Ŀ�Ķ˿ڷ�Χ��ʼֵ������0ʱ�����
 *	@usDstPortEnd: Ŀ�Ķ˿ڷ�Χ����ֵ������0ʱ�����
 *	@ucProtocol: Э�����ͣ�����0ʱ�����
 *	@ucDscp: DSCPֵ������0ʱ�����
 *	@acIntfName: �ӿ����ƣ��������򲻼��
 *	@lIfIndex: �ӿڶ�Ӧ�Ľӿ�����
 *	@acMac: MAC��ַ����Чʱת��Ϊ��Ӧ��IPv4��IPv6��ַ
 *	@ulIpv4Addrs: MAC��ַ��Ӧ������IPv4��ַ
 *	@stIpv6Addrs: MAC��ַ��Ӧ������IPv6��ַ
 *	@enDirection: ��鷽��,LAN or WAN
 *	@lBrIfIndex: ��ϼ�鷽��,��ӦLAN������bridge�ӿڵ�����
 *	@lBrNum: ��ϼ�鷽��,LAN bridge����
 *	@enCtType: �ڲ����ͣ���Ҫbondingʹ��
 */
typedef struct tagATP_COMMON_CT_CLEAN_INFO
{
    unsigned int    ulSrcIpStart;
    unsigned int    ulSrcIpEnd;
    unsigned int    ulSrcMask;
    unsigned int    ulDstIpStart;
    unsigned int    ulDstIpEnd;
    unsigned int    ulDstMask;
    struct in6_addr	stSrcIp6Start;
    struct in6_addr	stSrcIp6End;
    struct in6_addr	stSrcMask6;
    struct in6_addr	stDstIp6Start;
    struct in6_addr	stDstIp6End;
    struct in6_addr	stDstMask6;
    unsigned short  usSrcPortStart;
    unsigned short  usSrcPortEnd;
    unsigned short  usDstPortStart;
    unsigned short  usDstPortEnd;
    unsigned char   ucProtocol;
    unsigned char   ucDscp;
    char            acIntfName[ATP_COMMON_CT_CLEAN_INTF_LEN];
    int             lIfIndex;
    char            acMac[ATP_COMMON_CT_CLEAN_MAC_LEN];
    unsigned int    ulIpv4Addrs[ATP_COMMON_MAX_IP_PER_MAC_NUM];
    struct in6_addr stIpv6Addrs[ATP_COMMON_MAX_IP_PER_MAC_NUM];
    ATP_COMMON_CT_CLEAN_DIRECTION   enDirection;  /*LAN�෢����WAN�෢��,ACLʹ��*/
    int             lBrIfIndex[ATP_COMMON_CT_CLEAN_BR_MAX];
    int             lBrNum;
    ATP_COMMON_CT_CLEAN_TYPE enCtType;
} ATP_COMMON_CT_CLEAN_INFO;

#ifndef __KERNEL__


VOS_VOID ATP_COMMON_CtClean(ATP_COMMON_CT_CLEAN_INFO* pstCleanInfo);



VOS_VOID ATP_COMMON_CtCleanWithReserve(ATP_COMMON_CT_CLEAN_INFO* pstCleanInfos,
                                       VOS_UINT32 ulCleanNum);



VOS_UINT32 ATP_COMMON_CtCleanByMac(const VOS_CHAR* pszMacAddr);



VOS_UINT32 ATP_COMMON_CtCleanByInterface(const VOS_CHAR* pszIntfName);



VOS_UINT32 ATP_COMMON_GetAllIpFromMac(const VOS_CHAR* pszMac,
                                      VOS_UINT32* pulIpv4Addrs,
                                      VOS_UINT32 ulIpv4AddrsSize,
                                      struct in6_addr* pstIpv6Addrs,
                                      VOS_UINT32 ulIpv6AddrsSize);


VOS_VOID ATP_COMMON_FlushAccelCache();


VOS_VOID ATP_COMMON_CtClean_Debug(VOS_UINT32 ulArgc, VOS_CHAR*  acLastPara[]);



VOS_UINT8 ATP_COMMON_ProtoStr2Int(const VOS_CHAR* pszProtocol);



VOS_UINT32 ATP_COMMON_Ipv4Str2Int(const VOS_CHAR* pszIpAddr);



struct in6_addr ATP_COMMON_Ipv6Str2In6(const VOS_CHAR* pszIpAddr);

#endif

#ifdef SUPPORT_ATP_HYBRID_BONDING
#define ATP_CONNTRACK_BYPASS_MARK 0x20   /*Bypass ������mark*/
#define ATP_CONNTRACK_TUNNEL_MARK 0x40  /*���������mark*/
#define ATP_CONNTRACK_FLOW_MARK_MASK 0x60 /*����mark������*/
#endif

#ifdef __cplusplus
}
#endif

#endif

