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

/*****************************************************************************/
/*                                                                           */
/*                Copyright 1999 - 2003, Huawei Tech. Co., Ltd.              */
/*                           ALL RIGHTS RESERVED                             */
/*                                                                           */
/* FileName: nvchar.c                                                        */
/*                                                                           */
/* Author: yuyangyang                                                        */
/*                                                                           */
/* Version: 1.0                                                              */
/*                                                                           */
/* Date: 2013-3-12                                                           */
/*                                                                           */
/* Description:   nv read&write user interface                               */
/*                                                                           */
/* Others:                                                                   */
/*                                                                           */
/* History:                                                                  */
/* 1. Date:                                                                  */
/*    Modification: Create this file                                         */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/



/*lint --e{533,830}*/
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/cdev.h>

#include "nv_comm.h"
#include "product_nv_id.h"
#include "SysNvId.h"

#define NVCHAR_DEVICE_NAME    "nv"
#define NVCHAR_DEVICE_CLASS   "nv_class"



#define NV_MAX_USER_BUFF_LEN      1024  /* �޸�Ϊ��Ӧ��ƥ�������û�buff�ߴ� */

struct nv_dev
{
    unsigned int nvid;
    struct cdev cdev;
};

typedef enum
{ 
    TYPE_MAC_ADDR = 0,   /*  mac��ַ*/ 
    TYPE_NDIS_STATE,  /*ndis��״̬*/ 
    TYPE_SD_MODE,     /*SD MODE*/ 
    TYPE_APP_INFO,    /*��½��Ϣ*/
    
    TYPE_SSID1_INFO,   /*SSID1*/ 
    TYPE_WIFI_PASSWD1_WPA, /*WIFI PASSWORD*/ 
    TYPE_WIFI_PASSWD1_WEP, /*WIFI PASSWORD*/ 
    
    TYPE_SSID2_INFO,   /*SSID2*/ 
    TYPE_WIFI_PASSWD2_WPA, /*WIFI PASSWORD*/ 
    TYPE_WIFI_PASSWD2_WEP, /*WIFI PASSWORD*/
    
    TYPE_APPIN_INFO,  /*�� CPEʹ��,���ڼ�¼ WIFI WPS��PIN��*/ 
    TYPE_GPS_ONOFF, /*GPS�Ƿ�ʹ��*/
    
    TYPE_MULT_WIFI_BASIC, /*V7R2 WIFI SSID*/
    TYPE_MULT_WIFI_SEC,   /*V7R2 WIFI KEY*/
    TYPE_FACTORY_MODE,    /*����ģʽ*/      
    TYPE_CPE_TELNET_SWITCH_NVIM_STRU,    /*��NV�����ڿ���CPE��Ʒtelnet�����Ƿ��*/

    /*����Ϊ��ͨƽ̨nv��balong���漰*/
    TYPE_FOTA_PPP_APN,            /* ����PPP����ʱ��APN */
    TYPE_FOTA_PPP_USER_ID,        /* ����PPP����������û��� */
    TYPE_FOTA_PPP_PASSWORD,       /* ����PPP������������� */
    TYPE_FOTA_PPP_AUTH,           /* ����PPP����ʱ���õļ�Ȩ��ʽ */
    TYPE_FOTA_SERVER_IP,          /* FOTA��������IP */
    TYPE_FOTA_SERVER_PORT,        /* FOTA�������Ķ˿ں� */
    TYPE_FOTA_MODE,               /* FOTAģʽ���趨 */
    TYPE_FOTA_DETECT_COUNT,       /* ���ؼ�������ֵ*/
    TYPE_FOTA_NWTIME,             /* ���ڲ�ѯ�Ļ�׼ʱ�� */
    TYPE_FOTA_TIMER,              /* ��ʱ���Ĳ��� */
    TYPE_FOTA_INTERVAL,           /* �ֶ��汾��ѯ�ļ��ʱ�� */
    TYPE_FOTA_SMS_CONFIG,         /* fota��δʹ�ã�Ԥ��*/
    TYPE_FOTA_TIME_STAMP,         /* fota��δʹ�ã�Ԥ��*/
    TYPE_FOTA_RSA_PUB_MODE,       /* fota��δʹ�ã�Ԥ��*/
    TYPE_FOTA_RSA_PUB_EXP,        /* fota��δʹ�ã�Ԥ��*/
    TYPE_FOTA_SMS_FLAG,  /* �ȱ�ʶ����FOTA ��־λ��Ҳ�����յ�FOTA ���ź���������� */
    /*����Ϊ��ͨƽ̨nv��balong���漰*/

    TYPE_NV_FACTORY_INFO_I,              /*���ڼ�¼���߹�λ�ļ����*/
    TYPE_NV_HUAWEI_BT_INFO_I,       /*33 �����豸��MAC��������Ϣ*/
    TYPE_DATA_LOCK_HASH_VAL,       /*�ն˳���˽������hashֵ32�ֽ�*/
    TYPE_WIFI_MAC_NUM,              /*35 wifi mac��ַ������TYPE_MAC_ADDRΪ ��mac��ַ��4�ֽ���������*/
    TYPE_LAN_MAC,                   /*36 LAN mac��ַ��ascci�ַ���ʽ�洢*/
    TYPE_LAN_MAC_NUM,               /*37 LAN mac��ַ������4�ֽ���������*/
    TYPE_TTS_CFG,                   /*38 TTS����*/
    TYPE_OMADM_IMS_PARAM,           /*39 OMADM����qp_ims_param_config�Ĳ���NV */
    TYPE_OMADM_IMS_SIP_EX0,         /*40 OMADM����qp_ims_sip_extended_0_config�Ĳ���NV */
    TYPE_OMADM_IMS_SMS,             /*41 OMADM����qp_ims_sms_config�Ĳ���NV */
    TYPE_OMADM_LTE_APN_SETTING,     /*42 OMADM config PDP profile*/
    TYPE_GPS_USER_SETTING_SAVED,    /*43 GPS �û��������� */
    TYPE_GPS_CFG_INFO,              /*44 GPS ������ò�����Ϣ */
    TYPE_GPS_CLIENT_CONFIG,         /*45 GPS client����*/
    TYPE_GDW_ENABLE_SWITCH ,        /*46 GDW Switch */
    TYPE_ENABLE_FOTA_SMS_AUTO_DOWNLOAD_FLAG,         /*47 ����FOTA�Զ����ع��ܵ�ʹ������*/
    TYPE_NV_WEB_ADMIN_PASSWORD = 50,/*50 webui�������*/
    TYPE_BT_MAC_NUM = 51,           /*51 Bluetooth mac��ַ������4�ֽ���������*/
    TYPE_USB_MAC_NUM = 52,          /*52 USB mac��ַ������4�ֽ���������*/
    TYPE_WIFI_2GPAVARS_INFO = 65,   /*65 WIFI����У׼�����2.4g��pavars����*/
    TYPE_WIFI_5GPAVARS_INFO = 66,   /*66 WIFI����У׼�����5g��pavars����*/
    TYPE_NV_MAX, 
}TYPE_KERNEL_NV_INFO; /* ��ö��ֵ�Ķ�����Ҫ����Ӧ�ò�Ķ��壬����ǰ��Ҫ��������Ӧ�ò����NV ID */
/*lint -e64 -e156 */
unsigned long nv_index_table[TYPE_NV_MAX + 1] = \
{
    NV_ID_MSP_MAC_ADDR,                        /* TYPE_MAC_ADDR   50014 */ 
    NV_ID_USB_TETHERING,      /* TYPE_NDIS_STATE   50524 */ 
    NV_ID_DRV_WEBNAS_SD_WORK_MODE,             /* TYPE_SD_MODE   51 */ 
    en_NV_Item_WEB_ADMIN_PASSWORD_NEW_I,       /* TYPE_APP_INFO   50418 */ 
    (unsigned long)NULL,                                      /* TYPE_SSID1_INFO   50290*/   /*50290-50295Ϊ��ͨ��NV��balong�ϲ�ʹ��*/         
    (unsigned long)NULL,                                      /* TYPE_WIFI_PASSWD1_WPA   50291 */ 
    (unsigned long)NULL,                                      /* TYPE_WIFI_PASSWD1_WEP   50292 */ 
    (unsigned long)NULL,                                      /* TYPE_SSID2_INFO   50293 */ 
    (unsigned long)NULL,                                      /* TYPE_WIFI_PASSWD2_WPA   50294 */ 
    (unsigned long)NULL,                                      /* TYPE_WIFI_PASSWD2_WEP   50295 */ 
    NV_ID_DRV_WPS_PIN,                         /* TYPE_APPIN_INFO   balongΪ0xCB8D*/ 
    (unsigned long)NULL,                                      /* TYPE_GPS_ONOFF   50280 */ 
    NV_ID_DRV_MULTI_WIFI_STATUS_SSID,          /* TYPE_MULT_WIFI_BASIC   9111 */
    NV_ID_DRV_MULTI_WIFI_KEY,                  /* TYPE_MULT_WIFI_SEC   9110 */
    NV_ID_DRV_VER_FLAG,                        /* TYPE_FACTORY_MODE   36 */
    NV_ID_TELNET_SWITCH_I,                     /* TYPE_CPE_TELNET_SWITCH_NVIM_STRU   50501 */
    NV_FOTA_PPP_APN,   /*16 TYPE_FOTA_PPP_APN 50400*/
    NV_FOTA_PPP_USER_ID,   /*17 TYPE_FOTA_PPP_USER_ID 50402*/
    NV_FOTA_PPP_PASSWORD,   /*18 TYPE_FOTA_PPP_PASSWORD 50401*/
    NV_FOTA_PPP_AUTHTYPE,   /*19 TYPE_FOTA_PPP_AUTH 50403*/
    NV_FOTA_SERVER_IP,   /*20 TYPE_FOTA_SERVER_IP 50404*/
    NV_FOTA_SERVER_PORT,   /*21 TYPE_FOTA_SERVER_PORT 50405*/
    NV_FOTA_DETECT_MODE,   /*22 TYPE_FOTA_MODE 50406*/
    NV_FOTA_DETECT_COUNT,   /*23 TYPE_FOTA_DETECT_COUNT 50407*/
    NV_FOTA_NWTIME,   /*24 TYPE_FOTA_NWTIME 50408*/
    NV_FOTA_TIMER,   /*25 TYPE_FOTA_TIMER 50409*/
    NV_FOTA_DETECT_INTERVAL,   /*26 TYPE_FOTA_INTERVAL 50447*/
    NV_Item_FOTA_SMS_CONFIG,   /*27 TYPE_FOTA_SMS_CONFIG 50470*/
    NV_Item_FOTA_TIME_STAMP,   /*28 TYPE_FOTA_TIME_STAMP 50471*/
    NV_Item_FOTA_RSA_PUB_MOD,   /*29 TYPE_FOTA_RSA_PUB_MODE 50472*/
    NV_Item_FOTA_RSA_PUB_EXP,   /*30 TYPE_FOTA_RSA_PUB_EXP 50473*/
    NV_FOTA_SMS_FLAG,   /*31 TYPE_FOTA_SMS_FLAG 50481*/
    /*����Ϊ��ͨƽ̨nv��balong���漰*/

    NV_ID_DRV_NV_FACTORY_INFO_I,               /* TYPE_NV_FACTORY_INFO_I   114 */
    NV_ID_MSP_MAC_ADDR,                        /* TYPE_NV_HUAWEI_BT_INFO_I   50014*/
    NV_ID_DATA_LOCK_I,                         /* TYPE_DATA_LOCK_HASH_VAL*/
    NV_ID_DRV_MAC_NUM,                         /* TYPE_WIFI_MAC_NUM  50517*/
    NV_ID_MSP_MAC_ADDR,                        /* TYPE_LAN_MAC       50014*/
    NV_ID_DRV_MAC_NUM,                         /* TYPE_LAN_MAC_NUM   50517*/
    NV_ID_TTS_CFG_I,                           /* NV_ID_TTS_CFG_I    50465   */
    (unsigned long)NULL,   /*39 TYPE_OMADM_IMS_PARAM*/
    (unsigned long)NULL,   /*40 TYPE_OMADM_IMS_SIP_EX0*/
    (unsigned long)NULL,   /*41 TYPE_OMADM_IMS_SMS*/
    (unsigned long)NULL,   /*42 TYPE_OMADM_LTE_APN_SETTING*/
    NV_Item_GPS_USER_SETTING,   /*43 TYPE_GPS_USER_SETTING_SAVED 50073*/
    NV_Item_GPS_SETTING,   /*44 TYPE_GPS_CFG_INFO 50280*/
    en_NV_Item_AT_CLIENT_CONFIG,   /*45 TYPE_GPS_CLIENT_CONFIG 9202*/
    (unsigned long)NULL/*nv�������*/, /*46 GDW Switch */
    (unsigned long)NULL/*nv�������*/,         /*47 ����FOTA�Զ����ع��ܵ�ʹ������50543  */
    (unsigned long)NULL/*nv�������*/,                         /*48 FOTA�ݽ��汾������*/
    (unsigned long)NULL/*nv��(unsigned long)�����*/,                         /*49 USB log���ƹ�����ӦNV 50498*/
    NV_ID_WEB_ADMIN_PASSWORD_NEW_I,            /*50 webui�������*/
    NV_ID_DRV_MAC_NUM,                         /*51 TYPE_BT_MAC_NUM  50517*/
    NV_ID_DRV_MAC_NUM,                         /*52 TYPE_USB_MAC_NUM  50517*/
    (unsigned long)NULL,                       /*53 ��ƽ̨С����*/
    (unsigned long)NULL,                       /*54 VSIM��������Ӳ��ģʽ*/
    (unsigned long)NULL,                       /*55 �������õĲ���*/
    (unsigned long)NULL,                       /*56 flash������������Ϣ*/
    (unsigned long)NULL,                       /*57 GPS����������Ϣ*/
    (unsigned long)NULL,                       /*58 GPS�û���������*/
    (unsigned long)NULL,                       /*59 TYPE_UBX_XO_VOLTAGE_CFG 50592*/
    (unsigned long)NULL,
    (unsigned long)NULL,
    (unsigned long)NULL,
    (unsigned long)NULL,
    (unsigned long)NULL,
    NV_ID_WIFI_2G_RFCAL,                       /*65 TYPE_WIFI_2GPAVARS_INFO 50601*/
    NV_ID_WIFI_5G_RFCAL,    	               /*66 TYPE_WIFI_5GPAVARS_INFO 50602*/
    (unsigned long)NULL,                                      /* TYPE_NV_MAX */
}; 
/*lint +e64 +e156 */

typedef struct
{
    unsigned char lanmac_num;        /* LAN_MAC/Cradle MAC����*/
    unsigned char wifimac_num;       /* WIFI_MAC ����*/
    unsigned char btmac_num;        /* BLUETOOTH_MAC����*/
    unsigned char usbmac_num;       /* USB_MAC����*/
    unsigned char reserve[4];          /*Ԥ���ֶ�*/
}NV_MAC_NUM_STRU;


/* �޸�Ϊ��Ӧ��ƥ��Ľṹ�� */
struct nv_data_stru
{
    TYPE_KERNEL_NV_INFO type;
    unsigned int        len;
    unsigned int        rst;
    unsigned char       data[NV_MAX_USER_BUFF_LEN];
};

#define NV_HEAD_LEN            (sizeof(struct nv_data_stru) - NV_MAX_USER_BUFF_LEN)

static struct nv_dev nv_cdev = {0};
static unsigned int nvchar_major;
static struct class* nv_class;
/*lint -save -e438*/
/*lint -save -e745 -e601 -e49 -e65 -e64 -e533 -e830*/
s32 nvdev_open(struct inode * inode, struct file * file);
s32 nvdev_release(struct inode * inode, struct file * file);
ssize_t nvdev_read(struct file *file, char __user *buf, size_t count,
			loff_t *ppos);
ssize_t nvdev_write(struct file *file, const char __user *buf, size_t count,
			loff_t *ppos);

static struct file_operations nv_fops = {
    .owner   = THIS_MODULE,
    .read    = nvdev_read,
    .write   = nvdev_write,
    .open    = nvdev_open,
    .release = nvdev_release,
};

s32 nvdev_open(struct inode *inode, struct file *file)/*lint !e830*/
{
    struct nv_dev* dev;

    dev = (struct nv_dev*)container_of(inode->i_cdev,struct nv_dev,cdev);
    file->private_data = dev;
    return 0;
}
s32 nvdev_release(struct inode *inode, struct file *file)
{
    inode = inode;
    file  = file;
    return 0;
}

#define MAC_STR_LEN    (17)    /*XX:XX:XX:XX:XX:XXʵ�ʳ�����12+5*/
#define MAC_HALF_NUM   (3 )  
#define MAC_ARRAY_LEN  (32) 
#define BITS_OF_BYTE   (8 )
#define BYTE_OF_NUM    (3 )    /*XX:�ĳ���*/
/************************************************************************
func: ��MAC(mac_input)��offset�󷵻ظ�mac_intput , mac��ʽ: XX:XX:XX:XX:XX:XX
input: mac_input: ��MAC   nv_len: �ϲ㴫������Ҫ��ȡNV�ĳ���
output: None
return: None
Example: 
AA:BB:CC:00:00:00 AA:BB:CC:00:00:01 AA:BB:CC:00:00:02 AA:BB:CC:00:00:03
AA:BB:CC:FF:FF:FE AA:BB:CC:FF:FF:FF AA:BB:CC:00:00:00 AA:BB:CC:00:00:01
AA:BB:CC:FF:FE:FE AA:BB:CC:FF:FE:FF AA:BB:CC:FF:FF:00 AA:BB:CC:FF:FF:01
************************************************************************/
/*lint -save -e160 -e506 -e522 -e527 */
void BaseMACToWLAN( char* mac_input, int offset, unsigned long nv_len )
{
    unsigned long temp = 0;
    char array_mac[MAC_ARRAY_LEN] = {0};
    int  cnt = 0;
    unsigned long low_mac = 0;
    char array_temp[4] = {0};

    if ( MAC_STR_LEN >= nv_len )
    {
        printk( KERN_ERR "%s, nv_len not enough: %lu", __func__, nv_len );
    }

    strncpy( array_mac, mac_input, MAC_STR_LEN ); 
   
    for ( cnt = 0; cnt < MAC_HALF_NUM; cnt++ )
    {
        (void)sscanf( ( char * )( array_mac + ( cnt + MAC_HALF_NUM ) * BYTE_OF_NUM ), "%lX", &temp ); 
        
        low_mac |= (unsigned long)( temp << ( ( MAC_HALF_NUM - cnt - 1 ) * BITS_OF_BYTE ) ); 
    }

    low_mac += offset;

    for ( cnt = 0; cnt < MAC_HALF_NUM; cnt++ )
    {
        temp = (unsigned char)( low_mac >> ( ( MAC_HALF_NUM - cnt - 1 ) * BITS_OF_BYTE ) );
        memset( array_temp, 0, sizeof(array_temp) );
        snprintf( array_temp, sizeof(array_temp), "%02X", (unsigned char)temp );
        memcpy( ( array_mac + ( cnt + MAC_HALF_NUM ) * BYTE_OF_NUM ), array_temp, 2 ); 
    }

    /*����ĩβ'\0'*/
    array_mac[MAC_STR_LEN] = 0;

    memcpy( mac_input, array_mac, MAC_STR_LEN + 1 );
}
/*lint -restore*/


/*****************************************************************************
 �� �� ��  : get_mac_offset
 ��������  : ��ȡ�豸��һ��mac��ַ����ڻ�mac��ַ��ƫ����
 �������  : type:�豸���ͣ�wifi��blooth��lan�ȣ�
             data:��ȡ��nv 50517���ֵ
 �������  : ��
 �� �� ֵ  : ƫ����
 ˵    ��  : ƫ�����ļ�������: nv 50014����һ����mac��ַBASE_MAC��nv50517����ÿ���豸
             mac�ĸ�����Ȼ�����¼���:
             LAN_MAC :[BASE_MAC,BASE_MAC+ lanmac_num)
             WIFI_MAC:[BASE_MAC+ lanmac_num,BASE_MAC+ lanmac_num+ wifimac_num)
             BLUETOOTH_MAC:
                      [BASE_MAC+lanmac_num+wifimac_num,
                       BASE_MAC+lanmac_num+wifimac_num+ btmac_num)
             USB_MAC:
                      [BASE_MAC+lanmac_num+wifimac_num+ btmac_num,
                       BASE_MAC+lanmac_num+wifimac_num+ btmac_num+ usbmac_num)  
             
*****************************************************************************/
int get_mac_offset(TYPE_KERNEL_NV_INFO type,void * data)
{
    int offset = 0;
    NV_MAC_NUM_STRU * mac_temp = data;
    switch(type)
    {
        case TYPE_LAN_MAC:
            offset = 0;
            break;
        case TYPE_MAC_ADDR:
            offset = mac_temp->lanmac_num;
            break;
        case TYPE_NV_HUAWEI_BT_INFO_I:
            offset = mac_temp->wifimac_num + mac_temp->lanmac_num;
            break;       
        default:
            break;
    }
    return offset;

}

/*****************************************************************************
 �� �� ��  : get_mac_num
 ��������  : ��ȡ�����豸mac ��ַ�ĸ���
 �������  : type:�豸���ͣ�wifi��blooth��lan�ȣ�
             data:��ȡ��nv 50517���ֵ
 �������  : ��
 �� �� ֵ  : mac��ַ����
 *****************************************************************************/
int get_mac_num(TYPE_KERNEL_NV_INFO type,void * data)
{
    int num = 0;
    NV_MAC_NUM_STRU * mac_temp = data;
    switch(type)
    {
        case TYPE_WIFI_MAC_NUM:
            num = mac_temp->wifimac_num;
            break;
        case TYPE_LAN_MAC_NUM:
            num = mac_temp->lanmac_num;
            break;
        case TYPE_BT_MAC_NUM:
            num = mac_temp->btmac_num;
            break;
        case TYPE_USB_MAC_NUM:
            num = mac_temp->usbmac_num;
            break;
        default:
            break;
    }
    return num;
}

/*lint -restore +e745 +e601 +e49 +e65 +e64 +e533 +e830*/
/*
data stru:
    ---------------------------------------------
    | NV ID | data length |     data      |
    ---------------------------------------------
*/

/*lint -save --e{529,527}*/
ssize_t nvdev_read(struct file *file,char __user *buf, size_t count,
			loff_t *ppos)
{
    ssize_t ret;
    struct nv_data_stru* kbuf = NULL;
    unsigned long   k_nvid = 0;
    unsigned long   k_nvlen = 0;
    unsigned long   mac_num = 0;
    unsigned char   mac_offset = 0; 
    NV_MAC_NUM_STRU nv_mac_num ={0};

    if((NULL == buf)||(count <= NV_HEAD_LEN)||(count > NV_MAX_USER_BUFF_LEN))/*lint !e737*/
    {
        return BSP_ERR_NV_INVALID_PARAM;
    }

    kbuf = (struct nv_data_stru*)kmalloc(count+1,GFP_KERNEL);
    if(NULL == kbuf)
    {
        return BSP_ERR_NV_MALLOC_FAIL;
    }

    if(copy_from_user(kbuf,buf,count))
    {
        kfree(kbuf);
        return -1;
    }
    /*BEGIN EUAP ��һ��*/

    if ( TYPE_NV_MAX <= kbuf->type )
    {
        printk( KERN_ERR "kbuf->type is err: %u", kbuf->type );
        kfree(kbuf);
        return -1;
    }
   
    k_nvid = nv_index_table[kbuf->type];
    
    if(TYPE_WIFI_MAC_NUM == kbuf->type || TYPE_LAN_MAC_NUM == kbuf->type 
        || TYPE_BT_MAC_NUM == kbuf->type || TYPE_USB_MAC_NUM == kbuf->type)
    {
        k_nvlen = sizeof(NV_MAC_NUM_STRU);
    }
    else
    {
        k_nvlen = count-(sizeof(TYPE_KERNEL_NV_INFO) + sizeof(unsigned int) + sizeof(unsigned int));
    }
    
    /*����NV��֧��ֱ�ӷ���*/
    if((0 == k_nvlen) || ((unsigned long )NULL == k_nvid))
    {
        printk(KERN_ERR "The request nv is not support here or para is err, kbuf->type = %d,count = %d\n", \
                         kbuf->type,count);
        kfree(kbuf);
        return -1;
    }

    /*����Ƕ�ȡmac��ַ���Ȼ�ȡmacƫ����*/
    if(TYPE_MAC_ADDR == kbuf->type || TYPE_NV_HUAWEI_BT_INFO_I == kbuf->type || TYPE_LAN_MAC == kbuf->type)
    {
        ret = (ssize_t)bsp_nvm_read(NV_ID_DRV_MAC_NUM,(u8*)&nv_mac_num,sizeof(NV_MAC_NUM_STRU));
        if(ret)
        {
            printk( KERN_ERR "[%s]nv read err!\n",__func__);
            kfree(kbuf);
            return ret;
        }      
        mac_offset = get_mac_offset(kbuf->type,&nv_mac_num);       
    }
    /* coverity[tainted_data] */
    ret = (ssize_t)bsp_nvm_read(k_nvid,kbuf->data,k_nvlen);
    kbuf->len = k_nvlen;
    if(ret)
    {
        kfree(kbuf);
        return ret;
    }

    kbuf->rst = NV_OK;


    if ( TYPE_MAC_ADDR == kbuf->type || TYPE_NV_HUAWEI_BT_INFO_I == kbuf->type || TYPE_LAN_MAC == kbuf->type )
    {
        BaseMACToWLAN( (char *)kbuf->data, mac_offset, k_nvlen );
    }

    else if(TYPE_WIFI_MAC_NUM == kbuf->type || TYPE_LAN_MAC_NUM == kbuf->type
        || TYPE_BT_MAC_NUM == kbuf->type || TYPE_USB_MAC_NUM == kbuf->type)
    {
        mac_num = get_mac_num(kbuf->type,kbuf->data);
        memset(kbuf->data, 0x0, (count + 1) - (sizeof(TYPE_KERNEL_NV_INFO) + sizeof(unsigned int) + sizeof(unsigned int)));
        kbuf->data[0] = mac_num;
        kbuf->len = 1;
        printk("[%s]kbuf->type=%d,mac_num=%d\n",__func__,kbuf->type,kbuf->data[0]);
    }
    
    ret = (ssize_t)copy_to_user(buf,kbuf,count);
    if(ret)
    {
        kfree(kbuf);
        return ret;
    }

    kfree(kbuf);
    return (ssize_t)count;
}
/*lint -restore*/

/*lint -save --e{529,527}*/
ssize_t nvdev_write(struct file *file,const char __user *buf, size_t count,
			loff_t *ppos)
{
    ssize_t ret;
    struct nv_data_stru* kbuf = NULL;
    unsigned long k_nvid = 0;
    unsigned long k_nvlen = 0;

    if((NULL == buf)||(count <= NV_HEAD_LEN)||(count > NV_MAX_USER_BUFF_LEN))/*lint !e737*/
    {
        printk("invalid parameter!\n");
        return BSP_ERR_NV_INVALID_PARAM;
    }

    kbuf = (struct nv_data_stru*)kmalloc(count+1,GFP_KERNEL);
    if(NULL == kbuf)
    {
        return BSP_ERR_NV_MALLOC_FAIL;
    }

    if(copy_from_user(kbuf,buf,count))
    {
        printk("copy from user fail!\n");
        kfree(kbuf);
        return -1;
    }
    /*BEGIN EUAP ��һ��*/

    if ( TYPE_NV_MAX <= kbuf->type )
    {
        printk( KERN_ERR "kbuf->nvid is err: %u", kbuf->type );
        kfree(kbuf);
        return -1;
    }
    
    k_nvid = nv_index_table[kbuf->type];
    k_nvlen = count-(sizeof(TYPE_KERNEL_NV_INFO) + sizeof(unsigned int) + sizeof(unsigned int));

    /*����NV��֧��ֱ�ӷ���*/
    if((0 == k_nvlen) ||((unsigned long )NULL == k_nvid))
    {
        printk(KERN_ERR "The request nv is not support here or para is err, kbuf->type = %d,count = %d\n", \
                         kbuf->type,count);
        kfree(kbuf);
        return -1;
    }

    if(kbuf->len > NV_MAX_USER_BUFF_LEN)
    {
        printk("kbuf->len too large!\n");
        kfree(kbuf);
        return BSP_ERR_NV_INVALID_PARAM;
    }
    /* coverity[tainted_data] */
    ret = (ssize_t)bsp_nvm_write(k_nvid,kbuf->data,k_nvlen);
    if(ret)
    {
        kfree(kbuf);
        return ret;
    }

    kfree(kbuf);
    return (ssize_t)count;
}
/*lint -restore*/
/*lint -save -e527 -e533*/
void nvchar_dev_setup_cdev(struct nv_dev* dev,struct file_operations* fops)
/*lint -restore*/
{
    int ret;
    unsigned int devno = MKDEV(nvchar_major,0);


    cdev_init(&dev->cdev,fops);
    dev->cdev.owner= THIS_MODULE;
    dev->cdev.ops = fops;

    ret = cdev_add(&dev->cdev,devno,1);
    if(ret)
    {
        printk("add nv fail!\n");
        return ;
    }

    nv_class = class_create(THIS_MODULE,NVCHAR_DEVICE_CLASS);
    if(IS_ERR(nv_class))
    {
        printk("class create failed!\n");
        return ;
    }

    device_create(nv_class, NULL, devno, NULL, NVCHAR_DEVICE_NAME);
}



/*nv device init*/
/*lint -save --e{529}*/
int nvchar_init(void)
{
    int ret;
    dev_t dev = 0;

    /*dynamic dev num use*/
    ret = alloc_chrdev_region(&dev,0,1,NVCHAR_DEVICE_NAME);
    nvchar_major = MAJOR(dev);

    if(ret)
    {
        printk("nvchar failed alloc :%d\n",nvchar_major);
        return ret;
    }
    /*setup dev*/
    nvchar_dev_setup_cdev(&nv_cdev,&nv_fops);

    printk("nvchar init end! major: 0x%x\n",nvchar_major);
    return NV_OK;
}
/*lint -restore*/




MODULE_AUTHOR("Hisilicon Drive Group");
MODULE_DESCRIPTION("Balong NV driver for Hisilicon");
MODULE_LICENSE("GPL");

/*lint -restore*/

