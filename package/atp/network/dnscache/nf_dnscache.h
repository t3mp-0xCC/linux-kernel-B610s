#ifndef _NF_DNSCACHE_H
#define _NF_DNSCACHE_H

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/debugfs.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netfilter_ipv6.h>
#include <linux/in.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <net/ipv6.h>
#include <net/ip.h>

#define MATCHIP             1
#define NOMATCHIP           0
#define BUF_STRING_LEN      1024
#define URL_STRING_LEN      256
#define LANINTERFACE    "br"
#define LOCALINTERFACE  "lo"
#define FILTERTYPEBLACK  0
#define FILTERTYPEWHITE  1

#define TURNBRIDAGE      2  /*!< ��ģʽ���� ������DNS cache Filter*/
#define TURNON           1
#define TURNOFF          0

#define NIP6(addr) \
    ntohs((addr).s6_addr16[0]), \
    ntohs((addr).s6_addr16[1]), \
    ntohs((addr).s6_addr16[2]), \
    ntohs((addr).s6_addr16[3]), \
    ntohs((addr).s6_addr16[4]), \
    ntohs((addr).s6_addr16[5]), \
    ntohs((addr).s6_addr16[6]), \
    ntohs((addr).s6_addr16[7])

#define NIPQUAD(addr) \
	((unsigned char *)&addr)[0], \
	((unsigned char *)&addr)[1], \
	((unsigned char *)&addr)[2], \
	((unsigned char *)&addr)[3]	
	


//����ڵ�
struct dnsipv4info
{
    unsigned int              dipv4;             //IPv4ȡֵ   
    struct list_head         ipv4_list;
};
 
struct dnsipv6info
{   
    struct in6_addr          addrv6;           //IPv6ȡֵ
    struct list_head         ipv6_list;
};
//dns����
struct st_Domainmap_Record
{
    struct hlist_node         i_hash;
    struct list_head          i_listv4;  
    struct list_head          i_listv6;  
    unsigned char*             pacUrl;           /* ����*/
    long                       ts;               //ʱ�����update��
    unsigned int               ulTTL;           //TTLʱ��s
};

//ip����ƥ��
struct st_DomainIP_Record
{
    struct list_head                list;                               /* ָ����һ��*/
    struct st_Domainmap_Record      *pnodeinfo;
    char *pmac;                                                         /*��url��Ӧ���豸mac*/
};

typedef struct tagDNS_RECORDS_ST
{
    unsigned char   acName[URL_STRING_LEN];
    unsigned int    ulType;
    unsigned int    ulClass;
    unsigned int    ulTTL;
    unsigned int    ulDataLen;
    const unsigned char *pucData;
} DNS_RECORDS_ST;


//int Get_Domaincache_by_Name(char *purl, struct dnsipinfo *pinfo);

/*ͨ���������Ҷ�Ӧ��ipv4��ַ*/
int dnscache_getipv4_by_Name(char *purl, struct list_head **pipv4);

int judge_ipcache_match( unsigned int ip,char *pmac);

int judge_ipcache_matchv6(struct in6_addr* pipv6,char *pmac);
#endif
