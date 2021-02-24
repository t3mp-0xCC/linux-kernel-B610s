#ifndef _ATP_PRSITE_H_
#define _ATP_PRSITE_H_

#include <linux/timer.h>
#include <linux/spinlock.h>
#include <linux/list.h>
#include <asm/atomic.h>
#include <linux/netfilter.h>
#include <linux/netfilter/nf_conntrack_tuple_common.h>
#include <net/netfilter/nf_conntrack.h>

#define HTTP_PORT 80

/* start of 防止用户态配置下来的字符串长度加上 http:// */ 
#define HTTP_URL_MAX (256 + 32)
/* end of 防止用户态配置下来的字符串长度加上 http://  */
#define HTTP_TRACE_CHECK_TIMEOUT 5 //minutes
#define HTTP_TRACE_TIMEOUT 30 //minutes
#define HTTP_PROTO_HEAD_BUF_MAX 16
#define HTTP_RESPONSE_BUF_MAX (HTTP_URL_MAX + 256)
#define AFFINED_ADDR_BUF_MAX 16

#define AFFINED_ADDR_STATIC_BASE 8

#define HTTP_PROTO_NAME "HTTP"
#define HTTP_PROTO_GET  "GET"



struct prsite_url_info
{
    int          lEnable;
    unsigned int ul_lan_addr;
    unsigned int ul_lan_mask;
    char         ac_stb_url[HTTP_URL_MAX];
#if defined(CONFIG_ATP_GUESTWIFI_ACCESS_UI) && defined(BSP_CONFIG_BOARD_E5_SB03)    
    char         ac_stb_url_guestwifi[HTTP_URL_MAX];
#endif
    char         ac_computer_url[HTTP_URL_MAX];
    char         ac_wifi_url[HTTP_URL_MAX];
    int          lMultiuserEnable;//multimac
};

//for ioctl
struct affined_bind
{
    unsigned int addr;
    unsigned int mask;
    unsigned int flag;
    char         url[HTTP_URL_MAX];
};
struct user_agent_list
{
    char *key_type;
    char *key_version;
    int version_num;
};
void delrandall(void);
int showrandall(char *buffer, size_t len);
void delrandbyid(char *pid);

//multimac
void showmaclist();
void delmaclistbymac(char *mac);

#endif
