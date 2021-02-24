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

#include <linux/types.h>
#include <linux/netfilter/nf_conntrack_tuple_common.h>
#include "spe_sim_entry.c"

struct spe_sim_mac_fw_entry st_usb_entry=
{
	.mac={0x0a,0x5b,0x8f,0x27,0x9a,0x64},
	.br_portno=9,/*cpu port? how to fill this?*/
	.reserve=0,
	.port_index=1,
};

struct spe_sim_mac_fw_entry st_eth_entry=
{
	.mac={0x58,0x02,0x03,0x04,0x05,0x06},
	.br_portno=9,/*cpu port? how to fill this?*/
	.reserve=0,
	.port_index=2,
};


struct spe_sim_mac_fw_entry new_ut_entry1=
{
	.mac={0xae,0xae,0xae,0xae,0xae,0xae},
	.br_portno=9,/*cpu port? how to fill this?*/
	.reserve=0,
	.port_index=1,
};

struct spe_sim_mac_fw_entry new_ut_entry2=
{
	.mac={0xaf,0xaf,0xaf,0xaf,0xaf,0xaf},
	.br_portno=10,/*cpu port? how to fill this?*/
	.reserve=0,
	.port_index=2,
};

struct spe_sim_mac_fw_entry src_ut_entry=
{
	.mac={0x00,0x1B,0x78,0x40,0x08,0x1E},
	.br_portno=9,/*cpu port? how to fill this?*/
	.reserve=0,
	.port_index=1,
};
/*Is a under bridge*/
struct spe_sim_mac_fw_entry dst_ut_entry_is_bridge=
{
	.mac={0x00,0x1B,0x78,0x40,0x08,0x1F},
	.br_portno=10,/*cpu port? how to fill this?*/
	.reserve=0,
	.port_index=10,
};

/*under bridge*/
struct spe_sim_mac_fw_entry dst_ut_entry_under_bridge=
{
	.mac={0x00,0x1B,0x78,0x40,0x08,0x1F},
	.br_portno=10,/*cpu port? how to fill this?*/
	.reserve=0,
	.port_index=2,
};

struct spe_sim_mac_fw_entry src_ut_entry_under_bridge=
{
	.mac={0x00,0x1B,0x78,0x40,0x08,0x1E},
	.br_portno=10,/*cpu port? how to fill this?*/
	.reserve=0,
	.port_index=1,
};


struct spe_sim_mac_fw_entry src_ut_entry_fake=
{
	.mac={0x00,0x1B,0x78,0x40,0x08,0x1E},
	.br_portno=10,/*cpu port? how to fill this?*/
	.reserve=0,
	.port_index=2,
};




/* UDP packet */
#pragma pack(1)
#define ETH_MAC_ADDR_LEN                6
#define ETH_MAC_HEADER_LEN                14

/* IP V4地址的字节数 */
#define IP_ADDR_LEN    4

#define IP_HDR_LEN     20

#define UDP_HDR_LEN    8

#define IP_PAYLOAD     8


#define ETH_HTONS(x)    ((((x) & 0x00ff) << 8) | \
                        (((x) & 0xff00) >> 8))


static unsigned char g_aucSrcMac[ETH_MAC_ADDR_LEN] = {0x00,0x1B,0x78,0x40,0x08,0x1E};
static unsigned char g_aucDstMac[ETH_MAC_ADDR_LEN] = {0x00,0x1B,0x78,0x40,0x08,0x1F};
static unsigned char g_aucDstIPAddr[4] = {192,168,1,100};
static unsigned char g_aucSrcIPAddr[4] = {192,168,1,1};
static unsigned short g_usSrcPort = 6001;
static unsigned short g_usDstPort = 6002;


/*port 1:(like gmac or usb connect pc address are for pc)
	under bridge 10 
	ip:192.168.1.10
	port:65533
	mac:0x00,0x1B,0x78,0x40,0x08,0x1F
	dst of dl
*/
/*port 2:(WAN&rnic, it has two port :2 and 9)
	under bridge 9 
	ip:202.118.80.200
	port:
	mac:0xAE,0xAE,0xAE,0xAE,0xAE,0xAE
*/
/*port 9:(WAN, the same thing to port 2, add this so spe can see rnic as a bridge)
	bridge
	ip:202.118.80.200
	port:65000
*/

/*port 10:(LAN, has no mac)
	bridge
	ip:192.168.1.1
	port:NO
	mac:0x00,0x1B,0x78,0x40,0x08,0x1E
*/
/*remote server
	ip:202.118.80.1
	port:60000
	mac:0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,
*/

static unsigned char g_local_dev_ip_br[IP_ADDR_LEN] = {192,168,1,11};
static unsigned char g_local_dev_ip_no_br[IP_ADDR_LEN] = {192,168,1,10};
static unsigned char g_local_lan_ip[IP_ADDR_LEN] = {192,168,1,1};
static unsigned char g_local_wan_ip[IP_ADDR_LEN] = {202,118,80,200};
static unsigned char g_remote_sever_ip[IP_ADDR_LEN] = {202,118,80,1};

static unsigned char g_local_dev_mac[MAC_ADDR_LEN]={0x00,0x1B,0x78,0x40,0x08,0x1F};
static unsigned char g_local_lan_mac[MAC_ADDR_LEN]={0x00,0x1B,0x78,0x40,0x08,0x1E};
static unsigned char g_local_wan_mac[MAC_ADDR_LEN]={0xAE,0xAE,0xAE,0xAE,0xAE,0xAE};
static unsigned char g_remote_sever_mac[MAC_ADDR_LEN]={0xAF,0xAF,0xAF,0xAF,0xAF,0xAF};

struct spe_sim_ip_fw_entry ip_dl_ut_entry_no_br;
struct spe_sim_ip_fw_entry ip_dl_ut_entry_br;
struct spe_sim_ip_fw_entry ip_ul_ut_entry;
struct spe_sim_mac_fw_entry dl_ut_entry_wan=
{
	.mac={0xAE,0xAE,0xAE,0xAE,0xAE,0xAE},
	.br_portno=9,
	.reserve=0,
	.port_index=9,
};

struct spe_sim_mac_fw_entry dl_ut_entry_dev=
{
	.mac={0x00,0x1B,0x78,0x40,0x08,0x1F},
	.br_portno=10,
	.reserve=0,
	.port_index=1,
};
struct spe_sim_mac_fw_entry dl_ut_entry_lan=
{
	.mac={0x00,0x1B,0x78,0x40,0x08,0x1E},
	.br_portno=10,
	.reserve=0,
	.port_index=10,
};

struct spe_sim_mac_fw_entry dl_ut_entry_remote_sever=
{
	.mac={0xAF,0xAF,0xAF,0xAF,0xAF,0xAF},
	.br_portno=9,
	.reserve=0,
	.port_index=2,
};
typedef struct _IPHDR
{
    unsigned char    ucIpHdrLen:4;                 /* version */
    unsigned char    ucIpVer:4;
    unsigned char    ucServiceType;     /* type of service */
    unsigned short   usTotalLen;           /* total length */
    unsigned short   usIdentification;   /* identification */
    unsigned short   usOffset;               /* fragment offset field */
    unsigned char    ucTTL;                    /* time to live*/
    unsigned char    ucProtocol;            /* protocol */
    unsigned short   usCheckSum;        /* checksum */
    unsigned int   ulSrcAddr;
    unsigned int   ulDestAddr;          /* source and dest address */
}ETH_TEST_IPHDR_T;

typedef struct _IP_PACKET_FORMAT
{
    unsigned char          dstMacAddr[ETH_MAC_ADDR_LEN];
    unsigned char          srcMacAddr[ETH_MAC_ADDR_LEN];
    unsigned short         usType;

    /* IP头 */
    ETH_TEST_IPHDR_T     stSCTPHdr;

    /* UDP头 */
    unsigned short          srcPort;
    unsigned short          dstPort;
    unsigned short          udpLen;
    unsigned short          udpChecksum;

    unsigned int          ulBody;
}ETH_TEST_IP_PACKET_FORMAT_T;
#pragma pack(0)

unsigned short EthTestCRC16(unsigned char *pucData, unsigned short usSize)
{
    unsigned int ulCheckSum  = 0;
    unsigned short *pBuffer = (unsigned short *)pucData;;

    while(usSize > 1)
    {
        ulCheckSum += *pBuffer++;
        usSize     -= sizeof(unsigned short);
    }

    if (usSize)
    {
        ulCheckSum += *(unsigned char*)pBuffer;
    }
    ulCheckSum  = (ulCheckSum >> 16) + (ulCheckSum & 0xffff);
    ulCheckSum += (ulCheckSum >> 16);

    return (unsigned short)(~ulCheckSum);
}

void EthTestBuildIpHdr(ETH_TEST_IP_PACKET_FORMAT_T *pstPkt, unsigned int ulLen)
{
    static unsigned short usIdentification2 = 45556;
    unsigned int ulIpAddr;

    memcpy(pstPkt->dstMacAddr,g_aucDstMac, ETH_MAC_ADDR_LEN);
    memcpy(pstPkt->srcMacAddr,g_aucSrcMac, ETH_MAC_ADDR_LEN);

    /* 填写帧类型 */
    pstPkt->usType = IP_PAYLOAD;

    /* 填写IP头 */
    pstPkt->stSCTPHdr.ucIpVer          = 4;
    pstPkt->stSCTPHdr.ucIpHdrLen       = 5;
    pstPkt->stSCTPHdr.ucServiceType    = 0x10;
    pstPkt->stSCTPHdr.usTotalLen       = ETH_HTONS((unsigned short)ulLen + sizeof(ETH_TEST_IPHDR_T)+ UDP_HDR_LEN);
    pstPkt->stSCTPHdr.usIdentification = ETH_HTONS(usIdentification2);
    usIdentification2++;
    pstPkt->stSCTPHdr.usOffset         = ETH_HTONS(IP_DF);
    pstPkt->stSCTPHdr.ucTTL            = 0xFF;
    pstPkt->stSCTPHdr.ucProtocol       = 17;    /*UDP*/
    pstPkt->stSCTPHdr.usCheckSum       = 0;

    memcpy((unsigned char*)(&ulIpAddr),g_aucDstIPAddr,4);
    pstPkt->stSCTPHdr.ulDestAddr = ulIpAddr;
    memcpy((unsigned char*)(&ulIpAddr),g_aucSrcIPAddr,4);
    pstPkt->stSCTPHdr.ulSrcAddr = ulIpAddr;

    pstPkt->stSCTPHdr.usCheckSum       = EthTestCRC16((unsigned char *)&pstPkt->stSCTPHdr,IP_HDR_LEN);

    pstPkt->srcPort = ETH_HTONS(g_usSrcPort);
    pstPkt->dstPort = ETH_HTONS(g_usDstPort);

    pstPkt->udpLen  = ETH_HTONS((unsigned short)ulLen + UDP_HDR_LEN);
    pstPkt->udpChecksum = 0;

    return;
}

/*
* data: packet data ptr, not include include mac and type
* payload: payload len, not include mac and type
* value: 0: not fill value to packet, others: fill the value to packet
*/
void EthTestBuildPkt(char* data, unsigned int payload, unsigned char value)
{
    ETH_TEST_IP_PACKET_FORMAT_T *pstPkt;
    unsigned int ulHdrLen;

    if(NULL == data)
    {
        return ;
    }

    if (payload > 1400)
    {
        payload = 1400;
    }

    pstPkt = (ETH_TEST_IP_PACKET_FORMAT_T*)(data);
    EthTestBuildIpHdr(pstPkt, payload);
    ulHdrLen = ETH_MAC_HEADER_LEN + IP_HDR_LEN + UDP_HDR_LEN;

    if (value)
    {
        memset((data) + ulHdrLen, value, payload);
    }
}


struct tag_spe_sim_port *cpu_port_stub = NULL;
void print_pkt_head(char* pkt)
{

}
int spe_sim_reg_get_cpu_port(struct tag_spe_sim_port **cpu_port)/*if return is anything other than 0,then no pc port has been found*/
{
	struct tag_spe_sim_port *temp_cpu_port = cpu_port_stub;

	/*Force 0 to be cpu port*/
	if(NULL != temp_cpu_port){
		printk("no need to malloc cpu port temp_cpu_port  is 0x%x \n",(unsigned int)temp_cpu_port);
		*cpu_port = temp_cpu_port;
		return 0;
	}
	
	temp_cpu_port = kmalloc(sizeof(struct tag_spe_sim_port), GFP_ATOMIC);
	if(NULL == temp_cpu_port){
		printk("fail to malloc cpu port temp_cpu_port  is 0x%x \n",(unsigned int)temp_cpu_port);
		return -ENOMEM;
	}
	temp_cpu_port->port_num = 0;
	*cpu_port = temp_cpu_port;
	printk("malloc cpu port success cpu_port  is 0x%x \n",(unsigned int)*cpu_port);

	cpu_port_stub = &temp_cpu_port;
	
	return 0;
}


/*init ut entry test, init mac_fw_ctx and pkt stub*/
void spe_sim_mac_entry_ut_init(void)
{
	int result;
	spe_sim_mac_entry_process_init();
	struct tag_spe_sim_port *temp_cpu_port;

	result = spe_sim_reg_get_cpu_port(&temp_cpu_port);
	if(0!=result){
		printk("Port config error!Can not find pc port \n");
	}
	spe_sim_reg_set_cpu_port(temp_cpu_port);
}

int spe_sim_clean_all_mac_entry_wrong(void)
{
	struct spe_mac_fw_entry_list *f;
	struct list_head *list_head = &mac_fw_ctx.head.list; 
	unsigned int i;
	
	spin_lock(&mac_fw_ctx.spe_mac_fw_list_lock);
	list_for_each_entry(f, list_head, list){
		list_del(&f->list);
		if(NULL != f){
			kfree(f);
		}
		else{
			printk("spe_sim_clean_all_mac_entry recive a null pointer \n");
		}
	}
	spin_unlock(&mac_fw_ctx.spe_mac_fw_list_lock);
	return 0;
}

int spe_sim_clean_all_mac_entry(void)
{
	unsigned int result;
	result = spe_sim_del_mac_fw_entry(&new_ut_entry1);
	if(0 != result){
		printk("new_ut_entry1 not in queue \n");
	}
	result = spe_sim_del_mac_fw_entry(&new_ut_entry2);
	if(0 != result){
		printk("new_ut_entry2 not in queue \n");
	}
	result = spe_sim_del_mac_fw_entry(&src_ut_entry);
	if(0 != result){
		printk("src_ut_entry not in queue \n");
	}
	result = spe_sim_del_mac_fw_entry(&dst_ut_entry_is_bridge);
	if(0 != result){
		printk("dst_ut_entry_is_bridge not in queue \n");
	}
	result = spe_sim_del_mac_fw_entry(&dst_ut_entry_under_bridge);
	if(0 != result){
		printk("dst_ut_entry_under_bridge not in queue \n");
	}
	result = spe_sim_del_mac_fw_entry(&src_ut_entry_fake);
	if(0 != result){
		printk("src_ut_entry_fake not in queue \n");
	}
	return 0;
}


int spe_sim_show_all_mac_entry(void)
{
	struct spe_mac_fw_entry_list *f;
	struct list_head *list_head = &mac_fw_ctx.head.list; 
	unsigned int i;
	
	spin_lock(&mac_fw_ctx.spe_mac_fw_list_lock);
	list_for_each_entry(f, list_head, list){
		printk("port_index = %u br_portno = %u	", f->entry.port_index, f->entry.br_portno);
		printk("mac_addr = ");
		for(i=0; i<ETH_MAC_ADDR_LEN; i++){
			printk("%2x ", f->entry.mac[i]);
		}
		printk("\n");
	}
	spin_unlock(&mac_fw_ctx.spe_mac_fw_list_lock);
	return 0;
}

int spe_sim_show_one_mac_entry(char* current_mac_addr)
{
	unsigned int i;

		printk("mac_addr = ");
		for(i=0; i<ETH_MAC_ADDR_LEN; i++){
			printk("%2x ", current_mac_addr[i]);
		}
		printk("\n");
	return 0;
}


/*Add mac entry test and find this entry*/
int spe_sim_entry_ut_001(void)
{
	int result;
	struct spe_sim_mac_fw_entry * entry1=&new_ut_entry1;
	struct spe_sim_mac_fw_entry * entry2=&new_ut_entry2;
	struct spe_mac_fw_entry_list 	*current_entry = NULL;

	result = spe_sim_add_mac_fw_entry(entry1);
	if(0 != result){
		printk("Add entry1 fail! \n");
	}
	result = spe_sim_get_mac_fw_entry(entry1->mac, &current_entry);
	if(0 != result){
		printk("GET entry1 fail! \n");
	}
	if(current_entry->entry.port_index != entry1->port_index){
		printk("GET entry1 error! \n");
	}
		
	
	result = spe_sim_add_mac_fw_entry(entry2);
	if(0 != result){
		printk("Add entry2 fail! \n");
	}
	result = spe_sim_get_mac_fw_entry(entry2->mac, &current_entry);
	if(0 != result){
		printk("GET entry2 fail! \n");
	}
	if(current_entry->entry.port_index != entry2->port_index){
		printk("GET entry2 error! \n");
	}

	return 0;
}

/*Delete mac entry test*/
int spe_sim_entry_ut_002(void)
{
	int result;
	struct spe_sim_mac_fw_entry * entry1=&new_ut_entry1;
	struct spe_sim_mac_fw_entry * entry2=&new_ut_entry2;
	struct spe_mac_fw_entry_list 	*current_entry = NULL;

	result = spe_sim_del_mac_fw_entry(entry1);
	if(0 != result){
		printk("Del entry1 func return fail! \n");
	}
	result = spe_sim_get_mac_fw_entry(entry1->mac, &current_entry);
	if(0 == result){
		printk("Del entry1 fail it is still in queue! \n");
	}

	result = spe_sim_del_mac_fw_entry(entry2);
	if(0 != result){
		printk("Del entry2 func return fail! \n");
	}
	result = spe_sim_get_mac_fw_entry(entry2->mac, &current_entry);
	if(0 != result){
		printk("Del entry2 fail it is still in queue! \n");
	}

	return 0;

}

/*src port not under bridge, src port mac equals to dst mac, pkt goes to ip forwarding*/
int spe_sim_entry_ut_003(void)
{
	int result;
	unsigned int pkt_size =1500;
	char * pkt;
	spe_sim_port_t src_port_info={
		.port_num = 3,/*a port not under bridge. */
		.port_mac = {0x00,0x1B,0x78,0x40,0x08,0x1F},
	};
	
	printk("suppose to go to ip process\n");

	pkt = kmalloc(pkt_size, GFP_ATOMIC);
	if(NULL == pkt){
		printk("kmalloc failed. \n");
		return -ENXIO;
	}
	EthTestBuildPkt(pkt,1400, 0);

	result = spe_sim_mac_entry_process(&src_port_info, pkt, pkt_size);
	if(0 != result){
		printk("spe_sim_mac_entry_process failed \n");
		return result;
	}
	return 0;
}

/*src port not under bridge, src port mac NOT equals to dst mac, pkt goes to cpu*/
int spe_sim_entry_ut_004(void)
{
	int result;
	unsigned int pkt_size =1500;
	char * pkt;
	spe_sim_port_t src_port_info={
		.port_num = 3,/*a port not under bridge. */
		.port_mac = {0x00,0x00,0x00,0x00,0x00,0x00},
	};
	
	printk("dst port suppose to be 0(CPU port) \n");

	pkt = kmalloc(pkt_size, GFP_ATOMIC);
	if(NULL == pkt){
		printk("kmalloc failed. \n");
		return -ENXIO;
	}
	EthTestBuildPkt(pkt,1400, 0);

	result = spe_sim_mac_entry_process(&src_port_info, pkt, pkt_size);
	if(0 != result){
		printk("spe_sim_mac_entry_process failed \n");
		return result;
	}
	return 0;
}

/*Mac forward test to cpu, no dst mac entry no entry at all*/
int spe_sim_entry_ut_005(void)
{
	int result;
	unsigned int pkt_size =1500;
	char * pkt;
	spe_sim_port_t src_port_info={
		.port_num = 1,/*a port under bridge. */
		.port_mac = {0x00,0x1B,0x78,0x40,0x08,0x1E},/*src addr*/
	};
	
	printk("dst port suppose to be 0(CPU port) \n");

	/*Clean all mac entry. make sure there is no src entry*/
	spe_sim_clean_all_mac_entry();

	pkt = kmalloc(pkt_size, GFP_ATOMIC);
	if(NULL == pkt){
		printk("kmalloc failed. \n");
		return -ENXIO;
	}
	EthTestBuildPkt(pkt,1400, 0);

	result = spe_sim_mac_entry_process(&src_port_info, pkt, pkt_size);
	if(0 != result){
		printk("spe_sim_mac_entry_process failed \n");
		return result;
	}
	return 0;

}

/*Mac forward test to cpu, have dst mac entry no src mac entry*/
int spe_sim_entry_ut_006(void)
{
	int result;
	struct spe_sim_mac_fw_entry * dst_entry=&dst_ut_entry_under_bridge;
	unsigned int pkt_size =1500;
	char * pkt;
	spe_sim_port_t src_port_info={
		.port_num = 1,/*a port under bridge. */
		.port_mac = {0x00,0x1B,0x78,0x40,0x08,0x1E},/*src addr*/
	};

	/*Clean all mac entry. make sure there is no src entry*/
	spe_sim_clean_all_mac_entry();

	result = spe_sim_add_mac_fw_entry(dst_entry);
	if(0 != result){
		printk("Add dst fail! \n");
	}
	
	printk("dst port suppose to be 0(CPU port) \n");

	pkt = kmalloc(pkt_size, GFP_ATOMIC);
	if(NULL == pkt){
		printk("kmalloc failed. \n");
		return -ENXIO;
	}
	EthTestBuildPkt(pkt,1400, 0);

	result = spe_sim_mac_entry_process(&src_port_info, pkt, pkt_size);
	if(0 != result){
		printk("spe_sim_mac_entry_process failed \n");
		return result;
	}
	return 0;

}
/*有没有可能一张表里一个port对应多个mac?*/
/*Mac forward test to cpu, src port and src does not match*/
int spe_sim_entry_ut_007(void)
{

	int result;
	struct spe_sim_mac_fw_entry * src_entry=&src_ut_entry_fake;
	struct spe_sim_mac_fw_entry * dst_entry=&dst_ut_entry_under_bridge;

	/*Clean all mac entry. make sure there is no src entry*/
	spe_sim_clean_all_mac_entry();
	
	result = spe_sim_add_mac_fw_entry(dst_entry);
	if(0 != result){
		printk("Add dst fail! \n");
	}
	result = spe_sim_add_mac_fw_entry(src_entry);
	if(0 != result){
		printk("Add src fail! \n");
	}

	unsigned int pkt_size =1500;
	char * pkt;
	spe_sim_port_t src_port_info={
		.port_num = 1,/*a port under bridge. */
		.port_mac = {0x00,0x1B,0x78,0x40,0x08,0x1E},/*src addr*/
	};
	
	printk("dst port suppose to be 0(CPU port) \n");

	pkt = kmalloc(pkt_size, GFP_ATOMIC);
	if(NULL == pkt){
		printk("kmalloc failed. \n");
		return -ENXIO;
	}
	EthTestBuildPkt(pkt,1400, 0);

	result = spe_sim_mac_entry_process(&src_port_info, pkt, pkt_size);
	if(0 != result){
		printk("spe_sim_mac_entry_process failed \n");
		return result;
	}
	return 0;

}

/*Mac forward test to cpu, dst is host port*/
int spe_sim_entry_ut_008(void)
{

	int result;
	struct spe_sim_mac_fw_entry * src_entry=&src_ut_entry;
	struct spe_sim_mac_fw_entry * dst_entry=&dst_ut_entry_under_bridge;


	/*Clean all mac entry. make sure there is no src entry*/
	spe_sim_clean_all_mac_entry();

	result = spe_sim_add_mac_fw_entry(dst_entry);
	if(0 != result){
		printk("Add dst fail! \n");
	}
	result = spe_sim_add_mac_fw_entry(src_entry);
	if(0 != result){
		printk("Add src fail! \n");
	}

	unsigned int pkt_size =1500;
	char * pkt;
	spe_sim_port_t src_port_info={
		.port_num = 1,/*host port. */
		.port_mac = {0x00,0x1B,0x78,0x40,0x08,0x1E},/*src addr*/
	};
	
	printk("dst port suppose to go to host port 0(cpu port) \n");

	pkt = kmalloc(pkt_size, GFP_ATOMIC);
	if(NULL == pkt){
		printk("kmalloc failed. \n");
		return -ENXIO;
	}
	EthTestBuildPkt(pkt,1400, 0);

	result = spe_sim_mac_entry_process(&src_port_info, pkt, pkt_size);
	if(0 != result){
		printk("spe_sim_mac_entry_process failed \n");
		return result;
	}
	return 0;



}

/*Mac forward test to other port, normal mac farwarding*/
int spe_sim_entry_ut_009(void)
{
	int result;
	struct spe_sim_mac_fw_entry * src_entry=&src_ut_entry_under_bridge;
	struct spe_sim_mac_fw_entry * dst_entry=&dst_ut_entry_under_bridge;


	/*Clean all mac entry. make sure there is no src entry*/
	spe_sim_clean_all_mac_entry();

	result = spe_sim_add_mac_fw_entry(dst_entry);
	if(0 != result){
		printk("Add dst fail! \n");
	}
	result = spe_sim_add_mac_fw_entry(src_entry);
	if(0 != result){
		printk("Add src fail! \n");
	}

	unsigned int pkt_size =1500;
	char * pkt;
	spe_sim_port_t src_port_info={
		.port_num = 1,/*host port. */
		.port_mac = {0x00,0x1B,0x78,0x40,0x08,0x1E},/*src addr*/
	};
	
	printk("dst port suppose to be 2(dst port) \n");

	pkt = kmalloc(pkt_size, GFP_ATOMIC);
	if(NULL == pkt){
		printk("kmalloc failed. \n");
		return -ENXIO;
	}
	EthTestBuildPkt(pkt,1400, 0);

	result = spe_sim_mac_entry_process(&src_port_info, pkt, pkt_size);
	if(0 != result){
		printk("spe_sim_mac_entry_process failed \n");
		return result;
	}
	return 0;

}

/*Mac forward test to ip forwarding*/
int spe_sim_entry_ut_010(void)
{
	int result;
	struct spe_sim_mac_fw_entry * src_entry=&src_ut_entry;
	struct spe_sim_mac_fw_entry * dst_entry=&dst_ut_entry_is_bridge;


	/*Clean all mac entry. make sure there is no src entry*/
	spe_sim_clean_all_mac_entry();

	result = spe_sim_add_mac_fw_entry(dst_entry);
	if(0 != result){
		printk("Add dst fail! \n");
	}
	result = spe_sim_add_mac_fw_entry(src_entry);
	if(0 != result){
		printk("Add src fail! \n");
	}

	unsigned int pkt_size =1500;
	char * pkt;
	spe_sim_port_t src_port_info={
		.port_num = 1,/*host port. */
		.port_mac = {0x00,0x1B,0x78,0x40,0x08,0x1E},/*src addr*/
	};
	
	printk("dst port suppose to go to IP fw process \n");

	pkt = kmalloc(pkt_size, GFP_ATOMIC);
	if(NULL == pkt){
		printk("kmalloc failed. \n");
		return -ENXIO;
	}
	EthTestBuildPkt(pkt,1400, 0);

	result = spe_sim_mac_entry_process(&src_port_info, pkt, pkt_size);
	if(0 != result){
		printk("spe_sim_mac_entry_process failed \n");
		return result;
	}
	return 0;

}

/*Dump mac entry test*/
int spe_sim_entry_ut_011(void)
{

}

/*Aging mac entry test*/
int spe_sim_entry_ut_012(void)
{

}


void spe_sim_ip_entry_ut_init(void)
{
	struct spe_sim_ip_fw_entry *entry_head = NULL;
	struct spe_sim_ip_fw_entry *ip_entry_head = NULL;
	unsigned int result;
	entry_head = kmalloc(sizeof(struct spe_sim_ip_fw_entry),GFP_ATOMIC);
	if(NULL == entry_head){
		printk("Malloc ip entry table head fail. \n");
		return;
	}
	memset((void *)entry_head, 0x0, sizeof(struct spe_sim_ip_forwarding));
	memset(&ip_dl_ut_entry_no_br, 0x0, sizeof(struct spe_sim_ip_fw_entry));
	memset(&ip_ul_ut_entry, 0x0, sizeof(struct spe_sim_ip_fw_entry));
	
	sep_sim_entry_set_hash_params((unsigned int)entry_head, 0, 1, sizeof(struct spe_sim_ip_fw_entry), 1);
	
	memcpy(&ip_dl_ut_entry_no_br.tuple.src.u3.ip, g_remote_sever_ip, IP_ADDR_LEN);/*外网服务器地址*/
	memcpy(&ip_dl_ut_entry_no_br.tuple.dst.u3.ip, g_local_wan_ip, IP_ADDR_LEN);
	memcpy(&ip_dl_ut_entry_no_br.nat.ip, g_local_dev_ip_no_br, IP_ADDR_LEN);

	memcpy(&ip_dl_ut_entry_no_br.dhost.octet, g_local_dev_mac, MAC_ADDR_LEN);
	memcpy(&ip_dl_ut_entry_no_br.shost.octet, g_local_lan_mac, MAC_ADDR_LEN);

	ip_dl_ut_entry_no_br.tuple.src.u.udp.port=ntohs(60000);
	ip_dl_ut_entry_no_br.tuple.src.l3num = PF_INET;
	ip_dl_ut_entry_no_br.tuple.dst.u.udp.port = ntohs(60001);
	ip_dl_ut_entry_no_br.port_index = 3;        /*dst port 3 not under br10 */
	ip_dl_ut_entry_no_br.nat.port = ntohs(65533);		/* manip data for SNAT, DNAT */
	ip_dl_ut_entry_no_br.action=1;                    /* nat action. 0:src nat 1:dst nat 2:not nat */
	ip_dl_ut_entry_no_br.iptype=0x0;                      /* [7]: 0:ipv4 1:ipv6 [6]: 0:dyn, 1:static */
	ip_dl_ut_entry_no_br.is_static=0;
	ip_dl_ut_entry_no_br.tuple.dst.protonum=17;    /*UDP*/;
	ip_dl_ut_entry_no_br.valid = 1;
	
	memcpy(&ip_dl_ut_entry_br.tuple.src.u3.ip, g_remote_sever_ip, IP_ADDR_LEN);/*外网服务器地址*/
	memcpy(&ip_dl_ut_entry_br.tuple.dst.u3.ip, g_local_wan_ip, IP_ADDR_LEN);
	memcpy(&ip_dl_ut_entry_br.nat.ip, g_local_dev_ip_br, IP_ADDR_LEN);

	memcpy(&ip_dl_ut_entry_br.dhost.octet, g_local_dev_mac, MAC_ADDR_LEN);
	memcpy(&ip_dl_ut_entry_br.shost.octet, g_local_lan_mac, MAC_ADDR_LEN);

	ip_dl_ut_entry_br.tuple.src.u.udp.port=ntohs(60000);
	ip_dl_ut_entry_br.tuple.src.l3num = PF_INET;
	ip_dl_ut_entry_br.tuple.dst.u.udp.port = ntohs(60002);
	ip_dl_ut_entry_br.port_index = 10;        /*dst port  br10 */
	ip_dl_ut_entry_br.nat.port = ntohs(55533);		/* manip data for SNAT, DNAT */
	ip_dl_ut_entry_br.action=1;                    /* nat action. 0:src nat 1:dst nat 2:not nat */
	ip_dl_ut_entry_br.iptype=0x0;                      /* [7]: 0:ipv4 1:ipv6 [6]: 0:dyn, 1:static */
	ip_dl_ut_entry_br.is_static=0;
	ip_dl_ut_entry_br.tuple.dst.protonum=17;    /*UDP*/;
	ip_dl_ut_entry_br.valid = 1;


	memcpy(&ip_ul_ut_entry.tuple.src.u3.ip, g_local_dev_ip_no_br, IP_ADDR_LEN);/*本地地址*/
	memcpy(&ip_ul_ut_entry.tuple.dst.u3.ip, g_local_wan_ip, IP_ADDR_LEN);
	memcpy(&ip_ul_ut_entry.nat.ip, g_remote_sever_ip, IP_ADDR_LEN);
	
	memcpy(&ip_ul_ut_entry.dhost.octet, g_remote_sever_mac, MAC_ADDR_LEN);
	memcpy(&ip_ul_ut_entry.shost.octet, g_local_wan_mac, MAC_ADDR_LEN);

	ip_ul_ut_entry.tuple.src.u.udp.port=ntohs(65533);
	ip_ul_ut_entry.tuple.src.l3num = PF_INET;
	ip_ul_ut_entry.tuple.dst.u.udp.port = ntohs(65000);
	ip_ul_ut_entry.port_index = 2;        /*dst port 1 under br10 */
	ip_ul_ut_entry.nat.port = ntohs(60000);		/* manip data for SNAT, DNAT */
	ip_ul_ut_entry.action=0;                    /* nat action. 0:src nat 1:dst nat 2:not nat */
	ip_ul_ut_entry.iptype=0x0;                      /* [7]: 0:ipv4 1:ipv6 [6]: 0:dyn, 1:static */
	ip_ul_ut_entry.is_static=0;
	ip_ul_ut_entry.tuple.dst.protonum=17;    /*UDP*/;
	ip_ul_ut_entry.valid = 1;

	
	result = spe_sim_add_mac_fw_entry(&dl_ut_entry_wan);
	if(0 != result){
		printk("Add entry wan fail! \n");
	}
	result = spe_sim_add_mac_fw_entry(&dl_ut_entry_lan);
	if(0 != result){
		printk("Add entry lan fail! \n");
	}
	result = spe_sim_add_mac_fw_entry(&dl_ut_entry_dev);
	if(0 != result){
		printk("Add entry dev fail! \n");
	}
	result = spe_sim_add_mac_fw_entry(&dl_ut_entry_remote_sever);
	if(0 != result){
		printk("Add entry remote server fail! \n");
	}

	
	/*add an ip entry*/
	ip_fw_ctx.entry_table =(void *)(&ip_ul_ut_entry);
	INIT_LIST_HEAD(&ip_ul_ut_entry.list);
	ip_entry_head = (struct spe_sim_ip_fw_entry *)ip_fw_ctx.entry_table;
	list_add(&ip_dl_ut_entry_br.list, &ip_entry_head->list);
	list_add(&ip_dl_ut_entry_no_br.list, &ip_entry_head->list);


}

void spe_sim_print_ip_entry_tuple(spe_sim_conn_tuple_t *tuple)
{	
	printk("src \n");
	printk("ip addr 0x%8x	",tuple->src.u3.ip);
	printk("port %5u	",ntohs(tuple->src.u.all));
	printk("l3 num %2u \n",tuple->src.l3num);

	printk("dst \n");
	printk("ip addr 0x%8x	",tuple->dst.u3.ip);
	printk("port %5u	",ntohs(tuple->dst.u.all));
	printk("l3 num %2u \n",tuple->dst.protonum);
	
}
void spe_sim_show_all_ip_entry(void)
{
	struct spe_sim_ip_fw_entry *head;
	struct spe_sim_ip_fw_entry *cur;
	unsigned int attr = 0;
	head = ip_fw_ctx.entry_table;
	
	/* 1. find tuple in header */
	spe_sim_print_ip_entry_tuple(&head->tuple);
	
	/* 2. find tuple in list */
	list_for_each_entry(cur, &(head->list), list) {
		if(NULL != cur)
		spe_sim_print_ip_entry_tuple(&cur->tuple);
	}
}


/*udp pkt, no entry in list, go to cpu*/
int spe_sim_entry_ut_101(void)
{
	int result;
	unsigned int pkt_size =1500;
	char * pkt;
	spe_sim_port_t src_port_info={
		.port_num = 1,/*a port under bridge. */
		.port_mac = {0x00,0x1B,0x78,0x40,0x08,0x1F},
	};
	
	pkt = kmalloc(pkt_size, GFP_ATOMIC);
	if(NULL == pkt){
		printk("kmalloc failed. \n");
		return -ENXIO;
	}
	EthTestBuildPkt(pkt,1400, 0);
	
    result = spe_sim_entry_eth_check(&src_port_info, pkt, pkt_size);
    if (result) {
	printk("spe_sim_entry_eth_check error!");
    }

	spe_sim_ip_entry_process(&src_port_info, pkt, pkt_size);

	return 0;

	

}

/*entry dst is bridge, can't find entry go to cpu maybe same to ut101?*/
int spe_sim_entry_ut_102(void)
{

}

/*dl, entry dst is bridge, can find entry, go to rd, check nat result*/
int spe_sim_entry_ut_103(void)
{
	int result;
	unsigned int pkt_size =1500;
	char * pkt;
	spe_sim_port_t src_port_info={
		.port_num = 2,/*a port under WAN bridge. */
		.port_mac = {0xAE,0xAE,0xAE,0xAE,0xAE,0xAE},
	};
	
	/*set pkt */

	memcpy(g_aucSrcMac, g_remote_sever_mac, MAC_ADDR_LEN);
	memcpy(g_aucDstMac, g_local_wan_mac, MAC_ADDR_LEN);
	memcpy(g_aucSrcIPAddr, g_remote_sever_ip, MAC_ADDR_LEN);
	memcpy(g_aucDstIPAddr, g_local_wan_ip, IP_ADDR_LEN);
	
	g_usSrcPort = 60000;
	g_usDstPort = 60002;
	pkt = kmalloc(pkt_size, GFP_ATOMIC);
	if(NULL == pkt){
		printk("kmalloc failed. \n");
		return -ENXIO;
	}
	EthTestBuildPkt(pkt,1400, 0);
   result = spe_sim_entry_eth_check(&src_port_info, pkt, pkt_size);
    if (result) {
	printk("spe_sim_entry_eth_check error!");
    }

	spe_sim_ip_entry_process(&src_port_info, pkt, pkt_size);

	return 0;

}

/*entry dst is not bridge, go to rd, check nat result*/
int spe_sim_entry_ut_104(void)
{
	int result;
	unsigned int pkt_size =1500;
	char * pkt;
	spe_sim_port_t src_port_info={
		.port_num = 2,/*a port under WAN bridge. */
		.port_mac = {0xAE,0xAE,0xAE,0xAE,0xAE,0xAE},
	};
	
	/*set pkt */

	memcpy(g_aucSrcMac, g_remote_sever_mac, MAC_ADDR_LEN);
	memcpy(g_aucDstMac, g_local_wan_mac, MAC_ADDR_LEN);
	memcpy(g_aucSrcIPAddr, g_remote_sever_ip, MAC_ADDR_LEN);
	memcpy(g_aucDstIPAddr, g_local_wan_ip, IP_ADDR_LEN);
	
	g_usSrcPort = 60000;
	g_usDstPort = 60001;
	pkt = kmalloc(pkt_size, GFP_ATOMIC);
	if(NULL == pkt){
		printk("kmalloc failed. \n");
		return -ENXIO;
	}
	EthTestBuildPkt(pkt,1400, 0);
   result = spe_sim_entry_eth_check(&src_port_info, pkt, pkt_size);
    if (result) {
	printk("spe_sim_entry_eth_check error!");
    }

	spe_sim_ip_entry_process(&src_port_info, pkt, pkt_size);

	return 0;
}

/*ip entry check fail*/
int spe_sim_entry_ut_105(void)
{

}

/*mac ip total test from port 2 to port 1*/
int spe_sim_entry_ut_200(void)
{
	int result;
	unsigned int pkt_size =1500;
	char * pkt;
	spe_sim_port_t src_port_info={
		.port_num = 2,/*a port under WAN bridge. */
		.port_mac = {0xAF,0xAF,0xAF,0xAF,0xAF,0xAF},
	};
	
	/*set pkt */

	memcpy(g_aucSrcMac, g_remote_sever_mac, MAC_ADDR_LEN);
	memcpy(g_aucDstMac, g_local_wan_mac, MAC_ADDR_LEN);
	memcpy(g_aucSrcIPAddr, g_remote_sever_ip, MAC_ADDR_LEN);
	memcpy(g_aucDstIPAddr, g_local_wan_ip, IP_ADDR_LEN);
	
	g_usSrcPort = 60000;
	g_usDstPort = 60002;
	pkt = kmalloc(pkt_size, GFP_ATOMIC);
	if(NULL == pkt){
		printk("kmalloc failed. \n");
		return -ENXIO;
	}
	EthTestBuildPkt(pkt,1400, 0);

	spe_sim_entry_process(&src_port_info, pkt, pkt_size);
	return 0;

}

void spe_set_rd(spe_sim_ctx_t *ctx)
{
	int i;
	reveived_desc_t* rd = ctx->ports[1].rd_ctx.base;
	for(i=0; i<63; i++){
		rd->attribute.irq_enable=1;
		rd->buf_addr_low=kzalloc(2000, GFP_ATOMIC);
		if(NULL == rd->buf_addr_low){
			printk("malloc rd[%d] fail \n", i);
		}
		rd=rd+1;
	}
	ctx->ports[1].rd_ctx.w_pos=63;
}

void spe_set_td(spe_sim_ctx_t *ctx)
{
	tran_desc_t* td = ctx->ports[2].td_ctx.base;
	int result;
	unsigned int pkt_size =1500;
	char * pkt;
	printk("before td set port 2 td wptr  = %u\n",ctx->ports[2].td_ctx.w_pos);
	printk("port 2 td base addr = 0x%x \n",ctx->ports[2].td_ctx.base);
	td = td + ctx->ports[2].td_ctx.w_pos;
	printk("port 2 td wptr addr = 0x%x \n",td);

	/*set pkt and td */
	memcpy(g_aucSrcMac, g_remote_sever_mac, MAC_ADDR_LEN);
	memcpy(g_aucDstMac, g_local_wan_mac, MAC_ADDR_LEN);
	memcpy(g_aucSrcIPAddr, g_remote_sever_ip, MAC_ADDR_LEN);
	memcpy(g_aucDstIPAddr, g_local_wan_ip, IP_ADDR_LEN);
	
	g_usSrcPort = 60000;
	g_usDstPort = 60002;
	pkt = kmalloc(pkt_size, GFP_ATOMIC);
	if(NULL == pkt){
		printk("kmalloc failed. \n");
		return -ENXIO;
	}
	EthTestBuildPkt(pkt,1400, 0);

	td->attribute.irq_enable=1;
	td->buf_addr_low = (unsigned int)pkt;
	td->pkt_len = 1500;
	
	if(ctx->ports[2].td_ctx.w_pos < 63)
		ctx->ports[2].td_ctx.w_pos++;
	else
		ctx->ports[2].td_ctx.w_pos=0;
	printk("port 2 cur_td.buf_addr = 0x%x \n", td->buf_addr_low);
	printk("after td set port 2 td wptr  = %u\n",ctx->ports[2].td_ctx.w_pos);
}




/*Add mac entry test and find this entry*/
int spe_sim_entry_st_stub(void)
{
	int result;
	struct spe_sim_mac_fw_entry * entry1=&st_usb_entry;
	struct spe_sim_mac_fw_entry * entry2=&st_eth_entry;
	struct spe_mac_fw_entry_list 	*current_entry = NULL;

	result = spe_sim_add_mac_fw_entry(entry1);
	if(0 != result){
		printk("Add entry1 fail! \n");
	}
	result = spe_sim_get_mac_fw_entry(entry1->mac, &current_entry);
	if(0 != result){
		printk("GET entry1 fail! \n");
	}
	if(current_entry->entry.port_index != entry1->port_index){
		printk("GET entry1 error! \n");
	}
		
	
	result = spe_sim_add_mac_fw_entry(entry2);
	if(0 != result){
		printk("Add entry2 fail! \n");
	}
	result = spe_sim_get_mac_fw_entry(entry2->mac, &current_entry);
	if(0 != result){
		printk("GET entry2 fail! \n");
	}
	if(current_entry->entry.port_index != entry2->port_index){
		printk("GET entry2 error! \n");
	}

	return 0;
}

