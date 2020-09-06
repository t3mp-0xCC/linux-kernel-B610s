
#ifndef _HW_CTCLEAN_H
#define _HW_CTCLEAN_H

#define DEBUG_ENABLE 1 //µ÷ÊÔ¿ª¹Ø
#ifdef DEBUG_ENABLE
#define DEBUGPC(str, args...) printk(str, ## args);
#else
#define DEBUGPC(str, args...)
#endif

#define MAX_MSGSIZE 1024
#define MAX_RULE_NUM 60

#define TCP_PROTOCOL 6
#define UDP_PROTOCOL 17
#define ESTABLISHED 3

typedef enum
{
    BREAK_ERROR,
    BREAK_DMZ,
    BREAK_PORTMAPPING,
    DELETE_CT
} INTERRUPT_TRANSMISSION_ACTION;

struct rule_info
{
    unsigned int interfaceIP;
    unsigned int lanhostIP;
    unsigned short startport;
    unsigned short endport;
    unsigned char protocol;
};

typedef struct tagATP_CT_CLEAN_INFO
{
    unsigned int action;
    unsigned int lanmask;
    unsigned int entrynum;
    struct rule_info ruleinfo[MAX_RULE_NUM];
}ATP_CT_CLEAN_INFO;


void hw_ctclean_process(struct sk_buff *skb);
#endif
