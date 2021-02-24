/******************************************************************************
  *         版权所有 (C), 2001-2020, 华为技术有限公司
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
#define DSCP_MARK_OFFSET 22/*根据不同的dscp打上不同的mark值，mark值第22~27位供dscp使用*/
#endif

#define ATP_COMMON_CT_CLEAN_INTF_LEN 32
#define ATP_COMMON_CT_CLEAN_MAC_LEN  18

#define ATP_COMMON_MAX_IP_PER_MAC_NUM  32

#define ATP_COMMON_CT_CLEAN_ARG_NUM  22

#define ATP_COMMON_CT_CLEAN_BR_MAX   16  /*接口查找br0:9也是一个bridge设备*/

#define ATP_COMMON_CT_CLEAN_MAX_IPV4 0xFFFFFFFF

#define ATP_COMMON_CT_CLEAN_TRIGGER_CMD  "ping -c 1 127.0.0.1 >/dev/null &"

#define ATP_COMMON_CT_CLEAN_HISI_CMD1    "hi_cli home/cli/debug/app/res/flushlink -v srvname notmc"
#define ATP_COMMON_CT_CLEAN_HISI_CMD2    "hi_cli home/cli/debug/app/res/flushlink -v srvname mc"
#define ATP_COMMON_CT_CLEAN_BCM_CMD      "fcctl flush"
#define ATP_COMMON_CT_CLEAN_BONDING_CMD  "cat proc/net/accel_flush"

typedef enum
{
    ATP_COMMON_CT_CLEAN_ALL       = 0,    /*所有的连接跟踪*/
    ATP_COMMON_CT_CLEAN_NONE      = (1 << 1),   /*不清连接跟踪*/
    ATP_COMMON_CT_CLEAN_BYPASS    = (1 << 2), /*走dsl bypass的连接跟踪*/
    ATP_COMMON_CT_CLEAN_BONDING   = (1 << 3),/*走bonding的连接跟踪*/
    ATP_COMMON_CT_CLEAN_ACL       = (1 << 4),    /*ACL类型*/
    ATP_COMMON_CT_CLEAN_ROUTE     = (1 << 5),  /*路由ADD*/
    ATP_COMMON_CT_CLEAN_PTTRIGGER = (1 << 6),
    ATP_COMMON_CT_CLEAN_FORWARD   = (1 << 7), /*LAN to WAN转发*/
} ATP_COMMON_CT_CLEAN_TYPE;

typedef enum
{
    ATP_COMMON_CT_CLEAN_DIRECTION_ALL,    /*不关注方向*/
    ATP_COMMON_CT_CLEAN_DIRECTION_LAN,     /*LAN侧发起的连接跟踪*/
    ATP_COMMON_CT_CLEAN_DIRECTION_WAN,     /*WAN侧发起的连接跟踪*/
} ATP_COMMON_CT_CLEAN_DIRECTION;


//!\brief 连接跟踪删除规则结构，用户态内核态通用
/*!
 * 	@ulSrcIpStart: IPv4源起始IP，输入0时不检查，否则检查匹配的源IP
 * 	@ulSrcIpEnd: IPv4源结束IP，输入0时不检查，否则检查匹配的源IP，忽略Mask
 *	@ulSrcMask: IPv4源IP对应掩码，输入0时不检查，否则和源起始IP配合检查掩码
 *	@ulDstIpStart: IPv4目的起始IP，输入0时不检查，否则检查匹配的目的IP
 *	@ulDstIpEnd: IPv4目的结束IP，输入0时不检查，否则检查匹配的目的IP，忽略Mask
 *	@ulDstMask: IPv4目的IP对应掩码，输入0时不检查，否则和目的IP配合检查掩码
 *    @stSrcIp6Start: IPv6源起始IP，输入0时不检查，否则检查匹配的源IP
 *    @stSrcIp6End: IPv6源结束IP，输入0时不检查，否则检查匹配的源IP，忽略Mask
 *	@stSrcMask6: IPv6源IP对应掩码，输入0时不检查，否则和源IP配合检查掩码
 *	@stDstIp6Start: IPv6目的起始IP，输入0时不检查，否则检查匹配的目的IP
 *	@stDstIp6End: IPv6目的结束糏P，输入0时不检查，否则检查匹配的目的IP，忽略Mask
 *	@stDstMask6: IPv6目的IP对应掩码，输入0时不检查，否则和目的IP配合检查掩码
 *	@usSrcPortStart: 源端口范围起始值，输入0时不检查
 *	@usSrcPortEnd: 源端口范围结束值，输入0时不检查
 *	@usDstPortStart: 目的端口范围起始值，输入0时不检查
 *	@usDstPortEnd: 目的端口范围结束值，输入0时不检查
 *	@ucProtocol: 协议类型，输入0时不检查
 *	@ucDscp: DSCP值，输入0时不检查
 *	@acIntfName: 接口名称，不设置则不检查
 *	@lIfIndex: 接口对应的接口索引
 *	@acMac: MAC地址，生效时转换为对应的IPv4和IPv6地址
 *	@ulIpv4Addrs: MAC地址对应的所有IPv4地址
 *	@stIpv6Addrs: MAC地址对应的所有IPv6地址
 *	@enDirection: 检查方向,LAN or WAN
 *	@lBrIfIndex: 配合检查方向,对应LAN侧所有bridge接口的索引
 *	@lBrNum: 配合检查方向,LAN bridge个数
 *	@enCtType: 内部类型，主要bonding使用
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
    ATP_COMMON_CT_CLEAN_DIRECTION   enDirection;  /*LAN侧发起还是WAN侧发起,ACL使用*/
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
#define ATP_CONNTRACK_BYPASS_MARK 0x20   /*Bypass 流量的mark*/
#define ATP_CONNTRACK_TUNNEL_MARK 0x40  /*隧道流量的mark*/
#define ATP_CONNTRACK_FLOW_MARK_MASK 0x60 /*流量mark的掩码*/
#endif

#ifdef __cplusplus
}
#endif

#endif

